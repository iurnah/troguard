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
	{ 0xd6de3455, "trace_event_raw_init" },
	{ 0x29cd3db7, "ftrace_event_reg" },
	{ 0x251016b9, "kthread_stop" },
	{ 0x8c096216, "wake_up_process" },
	{ 0x95272a9e, "kthread_create_on_node" },
	{ 0xf8acdc25, "trace_define_field" },
	{ 0xd2965f6f, "kthread_should_stop" },
	{ 0x27e1a049, "printk" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0xf5d0b8cb, "current_task" },
	{ 0xb4390f9a, "mcount" },
	{ 0x39e15e5f, "trace_nowake_buffer_unlock_commit" },
	{ 0x99226285, "filter_current_check_discard" },
	{ 0x9621849f, "ring_buffer_event_data" },
	{ 0xfef8a166, "trace_current_buffer_lock_reserve" },
	{ 0x4c4fef19, "kernel_stack" },
	{ 0x78764f4e, "pv_irq_ops" },
	{ 0x3e3bfe19, "perf_tp_event" },
	{ 0x7628f3c7, "this_cpu_off" },
	{ 0x7d11c268, "jiffies" },
	{ 0xe6602e9f, "perf_trace_buf_prepare" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x1e3a88fb, "trace_seq_printf" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "754C77A33148C658527E945");
