#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf6628fc9, "module_layout" },
	{ 0xc2cdbf1, "synchronize_sched" },
	{ 0xc11bd00f, "tracepoint_probe_unregister" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0xfa012fe7, "tracepoint_probe_register" },
	{ 0xb4d181ec, "path_put" },
	{ 0xd168aded, "dput" },
	{ 0xadaabe1b, "pv_lock_ops" },
	{ 0xd52bf1ce, "_raw_spin_lock" },
	{ 0xdb97f17, "path_get" },
	{ 0x27e1a049, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "38181A3689041F24ACBEE07");
