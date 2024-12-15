#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

static struct nf_hook_ops nat_ops;

unsigned int nat_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header = ip_hdr(skb);

    if (ip_header->protocol == IPPROTO_TCP || ip_header->protocol == IPPROTO_UDP) {
        
    }

    return NF_ACCEPT;
}

static int __init nat_init(void) {
    nat_ops.hook = nat_hook;
    nat_ops.pf = PF_INET;
    nat_ops.hooknum = NF_INET_POST_ROUTING;
    nat_ops.priority = NF_IP_PRI_FIRST;

    nf_register_hook(&nat_ops);
    return 0;
}

static void __exit nat_exit(void) {
    nf_unregister_hook(&nat_ops);
}

module_init(nat_init);
module_exit(nat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Den");
MODULE_DESCRIPTION("custom NAT netfilter hook");
