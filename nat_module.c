#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <nat_module.h>

#define PUB_IP 0x55566465//85.86.100.101 hexadecimal
#define PUB_PORT 443;

DEFINE_HASHTABLE(nat_table, 16);

static struct nf_hook_ops nat_ops_snat;
static struct nf_hook_ops nat_ops_dnat;

uint16_t assign_public_port(void) {
    static __be16 current_port = 10000;
    current_port++;
    if (current_port > 20000) 
        current_port = 10000;

    return htons(current_port);
}


static inline uint32_t hash_key(uint32_t ip, uint16_t port) {
    return jhash_2words(ip, port, 0);
}

static void add_nat_entry(uint32_t private_ip, uint16_t private_port,
                          uint32_t public_ip, uint16_t public_port) {
    struct nat_entry *entry = kmalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry)
        return;

    entry->private_ip = private_ip;
    entry->private_port = private_port;
    entry->public_ip = public_ip;
    entry->public_port = public_port;

    hash_add(nat_table, &entry->hnode, hash_key(public_ip, public_port));
}

uint8_t nat_outgoing_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header = ip_hdr(skb);
    struct tcphdr *tcp_header;
    struct udphdr *udp_header;

    if (ip_header->protocol == IPPROTO_TCP || ip_header->protocol == IPPROTO_UDP) {
        uint32_t src_ip = ip_header->saddr;
        ip_header->saddr = htonl(PUB_IP);
        csum_replace4(&ip_header->check, src_ip, htonl(PUB_IP));

        if(ip_header->protocol == IPPROTO_UDP) {
            udp_header = udp_hdr(skb);
            uint16_t src_port = udp_header->source;
            udp_header->source = assign_public_port();;

            csum_replace4(&udp_header->check, src_ip, htonl(PUB_IP));
            csum_replace4(&udp_header->check, src_port, tcp_header->source);
        }
        else if(ip_header->protocol == IPPROTO_TCP) {
            tcp_header = tcp_hdr(skb);
            uint16_t src_port = tcp_header->source;
            tcp_header->source = assign_public_port();;

            csum_replace4(&tcp_header->check, src_ip, htonl(PUB_IP));
            csum_replace4(&tcp_header->check, src_port, tcp_header->source);
        }
        add_nat_entry(src_ip, src_port, htonl(PUB_IP), PUB_PORT);
    }

    return NF_ACCEPT;
}

unsigned int nat_incoming_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header = ip_hdr(skb);
    struct tcphdr *tcp_header;
    struct udphdr *udp_header;
    struct nat_entry *entry;

    if (ip_header->protocol == IPPROTO_TCP || ip_header->protocol == IPPROTO_UDP) {
        uint32_t public_ip = ip_header->daddr;
        uint16_t public_port;

        if (ip_header->protocol == IPPROTO_TCP) {
            tcp_header = tcp_hdr(skb);
            public_port = tcp_header->dest;
        } else if (ip_header->protocol == IPPROTO_UDP) {
            udp_header = udp_hdr(skb);
            public_port = udp_header->dest;
        }

         hash_for_each_possible(nat_table, entry, hnode, hash_key(public_ip, public_port)) {
            if (entry->public_ip == public_ip && entry->public_port == public_port) {
                ip_header->daddr = entry->private_ip;
                csum_replace4(&ip_header->check, public_ip, entry->private_ip);

                if (ip_header->protocol == IPPROTO_TCP) {
                    tcp_header->dest = entry->private_port;
                    csum_replace4(&tcp_header->check, public_port, entry->private_port);
                } else if (ip_header->protocol == IPPROTO_UDP) {
                    udp_header->dest = entry->private_port;
                    csum_replace4(&udp_header->check, public_port, entry->private_port);
                }
                break;
            }
        }
    }

    return NF_ACCEPT;
}

static int __init nat_init(void) {
    nat_ops_snat.hook = nat_outgoing_hook;
    nat_ops_snat.pf = PF_INET;
    nat_ops_snat.hooknum = NF_INET_POST_ROUTING;
    nat_ops_snat.priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, &nat_ops_snat);

    nat_ops_dnat.hook = nat_incoming_hook;
    nat_ops_dnat.pf = PF_INET;
    nat_ops_dnat.hooknum = NF_INET_PRE_ROUTING;
    nat_ops_dnat.priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, &nat_ops_dnat);

    return 0;
}

static void __exit nat_exit(void) {
    struct nat_entry *entry;
    int bkt;

    nf_unregister_net_hook(&init_net, &nat_ops_snat);
    nf_unregister_net_hook(&init_net, &nat_ops_dnat);

    hash_for_each(nat_table, bkt, entry, hnode) {
        hash_del(&entry->hnode);
        kfree(entry);
    }
}


module_init(nat_init);
module_exit(nat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Den");
MODULE_DESCRIPTION("custom NAT netfilter hook");
