#include <linux/module.h>	// for init_module() 
#include <asm/uaccess.h>	// for copy_to/from_user()
#include <asm/unistd.h>		// for __NR_break
#include <asm/io.h>		// for phys_to_virt()
#include <asm/current.h> //for current
#include <linux/sched.h>	// for task_struct

#include <linux/fs.h>
#include <linux/namei.h>
#include <stdarg.h>
#include <linux/fdtable.h>

#include <linux/file.h>

#include <linux/net.h>
#include <net/inet_sock.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/pipe_fs_i.h>
#include <net/af_unix.h>

#include <linux/selinux.h>
#include <linux/security.h>

#include <linux/dcache.h>
#include <linux/syscalls.h>

#include <linux/kallsyms.h>
#include <asm/tlbflush.h>
#include <linux/ptrace.h>


#define WRITE_CMD "Write"
#define READ_CMD  "Read"

bool comment = false;


unsigned long *sys_call_table;

char modname[] = "newcall";
unsigned long save_old_pgtable_entry;
unsigned int _cr4, _cr3;
unsigned int *pgdir, dindex;
unsigned int *pgtbl, pindex;



char log_string[4096];
char log_string_1[128];


bool filtered(void){
	
	/*
	if (strstr(current->comm, "chrom"))
		return false;
	return true;
	*/
	if (strstr(current->comm, "syslog") ||
		//strstr(current->comm, "dhclient") ||
		//strstr(current->comm, "corehttp") ||
		//( !strcmp(current->comm, "php") && !strcmp(current->comm, "sh") && !strcmp(current->comm, "apache") ) || 
		//strstr(current->comm, "log") ||
		//strstr(current->comm, "avahi-daemon") ||
	 	strstr(current->comm, "geany") 
	 	)
		return true;
	return false;
}

// get the subjective security context of the current task
char *current_sec_context(){
	u32 sid,sid_len;
	char *sec_context;
	security_task_getsecid(current, &sid);
	security_secid_to_secctx(sid, &sec_context, &sid_len);
	return sec_context;
}

static int prepend(char **buffer, int *buflen, const char *str, int namelen)
{
	*buflen -= namelen;
	if (*buflen < 0)
		return -ENAMETOOLONG;
	*buffer -= namelen;
	memcpy(*buffer, str, namelen);
	return 0;
}

static int prepend_name(char **buffer, int *buflen, struct qstr *name)
{
	return prepend(buffer, buflen, name->name, name->len);
}


char *dentry_path(struct dentry *dentry, char *buf, int buflen)
{
	char *end = buf + buflen;
	char *retval;

	spin_lock(&dcache_lock);
	prepend(&end, &buflen, "\0", 1);
	if (d_unlinked(dentry) &&
		(prepend(&end, &buflen, "--deleted", 9) != 0))
			goto Elong;
	if (buflen < 1)
		goto Elong;
	// Get '/' right 
	retval = end-1;
	*retval = '/';

	while (!IS_ROOT(dentry)) {
		struct dentry *parent = dentry->d_parent;

		prefetch(parent);
		if ((prepend_name(&end, &buflen, &dentry->d_name) != 0) ||
		    (prepend(&end, &buflen, "/", 1) != 0))
			goto Elong;

		retval = end;
		dentry = parent;
	}
	spin_unlock(&dcache_lock);
	return retval;
Elong:
	spin_unlock(&dcache_lock);
	return ERR_PTR(-ENAMETOOLONG);
}


void log_fd(char command[], int fd){ // the process (current) does 'command' on 'fd'
	
	int length_check = 0;
	if (!filtered() && 2 < fd){
		
		strcpy(log_string, "");
		sprintf(log_string, "%s::Saman:: COMMAND: %s PROCESS: %s PID: %i SECCON: %s OBJTYPE: ", log_string, command, current->comm, current->pid, current_sec_context());
		
		struct files_struct *files = current->files;
		struct file *f = fcheck_files(files, fd);		
		
		if (f && f->f_dentry && f->f_dentry->d_inode && f->f_dentry->d_inode->i_mode){			
			
			length_check = strlen(log_string);
			
			if (S_ISREG(f->f_dentry->d_inode->i_mode)){ 
								
				
				sprintf(log_string, "%sREG FD: %i INODE: %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
				
				/*
				char fpath[350], fp2[350], tmpstr[150];	
				strcpy(fpath, "");
				struct dentry *d = f->f_dentry;
				
				if (d){
					strcpy(tmpstr, d->d_name.name);
					while (strcmp(tmpstr, "/")){
						
						strcpy(fp2, fpath);
						strcpy(fpath, "/");
						strcat(fpath, tmpstr);
						strcat(fpath, fp2);
						
						d = d->d_parent;
						if (!d || !strcmp(d->d_name.name, ""))
							break;
						strcpy(tmpstr, d->d_name.name);
					}
				}
				*/
				char buf[350];	
				char *fpath = dentry_path(f->f_dentry, buf, 350);
				
				/*
				char buf[256];
				strcpy(buf, "ERROR_dentry_path212");
				char *p = dentry_path(f->f_dentry, buf, 256);
				if (IS_ERR(p))
					strcpy(buf, "ERROR_dentry_path");
				sprintf(log_string, "%s%s extra:%s", log_string, fpath, p);
				*/

				sprintf(log_string, "%sFILEPATH: %s", log_string, fpath);
			
			}else if (S_ISDIR(f->f_dentry->d_inode->i_mode)){ 
				sprintf(log_string, "%sDIR %i %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
			}else if (S_ISLNK(f->f_dentry->d_inode->i_mode)){
				sprintf(log_string, "%sLNK %i %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
			}else if (S_ISCHR(f->f_dentry->d_inode->i_mode)){
				sprintf(log_string, "%sCHR %i %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
			}else if (S_ISBLK(f->f_dentry->d_inode->i_mode)){
				printk(KERN_ALERT "BLK %i %lu ", fd, f->f_dentry->d_inode->i_ino);
			}else if (S_ISFIFO(f->f_dentry->d_inode->i_mode)){
				sprintf(log_string, "%sFIFO FD: %i INODE: %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
				struct pipe_inode_info *pi = f->f_dentry->d_inode->i_pipe;
				if (pi)
					sprintf(log_string, "%sNRBUFS: %i CURBUF: %i ", log_string, pi->nrbufs, pi->curbuf);
				//printk("FIFO:: inode_num:%lu nrbuf:%i currbuf:%i readers:%i, writers:%i rc:%i wc:%i", f->f_dentry->d_inode->i_ino, pi->nrbufs, pi->curbuf, pi->readers, pi->writers, pi->r_counter, pi->w_counter);
				
			}else if (S_ISSOCK(f->f_dentry->d_inode->i_mode)){				
				struct socket *sockt = f->private_data;
				struct sock *sok = 0;
				if (sockt)
					sok = sockt->sk;
				if (sok && sok->sk_family){
					
					//char object_type[32];
					switch (sok->sk_family){ // descriptions in include/linux/socket.h
						
						
						case AF_UNIX: // same as AF_LOCAL
							sprintf(log_string, "%sSOCK_AF_UNIX FD: %i INODE: %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
							
							struct unix_sock *unix_sok = unix_sk(sok);
							if (unix_sok && unix_sok->peer && unix_sok->peer->sk_socket && unix_sok->peer->sk_socket->file && unix_sok->peer->sk_socket->file->f_dentry) // sometimes ...->sk_socket is NULL!
								sprintf(log_string, "%sINODE: %lu", log_string, unix_sok->peer->sk_socket->file->f_dentry->d_inode->i_ino);
							else 
								sprintf(log_string, "%sINODE: -1", log_string);
							
							//printk("\nAF_UNIX:: %lu %lu", sok->sk_socket->file->f_dentry->d_inode->i_ino, unix_sok->peer->sk_socket->file->f_dentry->d_inode->i_ino);
							break;
							
						case AF_INET:
							sprintf(log_string, "%sSOCK_AF_INET %i %lu ", log_string, fd, f->f_dentry->d_inode->i_ino);
							
							struct inet_sock *inet_sok = inet_sk(sok);
							
							if (inet_sok){
								
								u8 d_ip[4], s_ip[4];
								u16 s_port = 0, d_port = 0;
								
								s_port = inet_sok->sport;
								s_ip[0] = inet_sok->saddr & 0x000000ff;
								s_ip[1] = (inet_sok->saddr & 0x0000ff00) >> 8;
								s_ip[2] = (inet_sok->saddr & 0x00ff0000) >> 16;
								s_ip[3] = (inet_sok->saddr & 0xff000000) >> 24;
								
								d_port = inet_sok->dport;
								d_ip[0] = inet_sok->daddr & 0x000000ff;
								d_ip[1] = (inet_sok->daddr & 0x0000ff00) >> 8;
								d_ip[2] = (inet_sok->daddr & 0x00ff0000) >> 16;
								d_ip[3] = (inet_sok->daddr & 0xff000000) >> 24;	
								
								sprintf(log_string, "%s%i.%i.%i.%i:%i %i.%i.%i.%i:%i", log_string, s_ip[0], s_ip[1], s_ip[2], s_ip[3], s_port, d_ip[0], d_ip[1], d_ip[2], d_ip[3], d_port);						
								
								//printk("AF_INET:: %i.%i.%i.%i:%i to %i.%i.%i.%i:%i", s_ip[0], s_ip[1], s_ip[2], s_ip[3], inet_sok->sport, d_ip[0], d_ip[1], d_ip[2], d_ip[3], inet_sok->dport);
							}
							break;
						
						case AF_UNSPEC:sprintf(log_string, "%sSOCK_AF_UNSPEC ", log_string);break;
						case AF_AX25:sprintf(log_string, "%sSOCK_AF_AX25 ", log_string);break;
						case AF_IPX:sprintf(log_string, "%sSOCK_AF_IPX ", log_string);break;
						case AF_APPLETALK:sprintf(log_string, "%sSOCK_AF_APPLETALK ", log_string);break;
						case AF_NETROM:sprintf(log_string, "%sSOCK_AF_NETROM ", log_string);break;
						case AF_BRIDGE:sprintf(log_string, "%sSOCK_AF_BRIDGE ", log_string);break;
						case AF_ATMPVC:sprintf(log_string, "%sSOCK_AF_ATMPVC ", log_string);break;
						case AF_X25:sprintf(log_string, "%sSOCK_AF_X25 ", log_string);break;
						case AF_INET6:sprintf(log_string, "%sSOCK_AF_INET6 ", log_string);break;
						case AF_ROSE:sprintf(log_string, "%sSOCK_AF_ROSE ", log_string);break;
						case AF_DECnet:sprintf(log_string, "%sSOCK_AF_DECnet ", log_string);break;
						case AF_NETBEUI:sprintf(log_string, "%sSOCK_AF_NETBEUI ", log_string);break;
						case AF_SECURITY:sprintf(log_string, "%sSOCK_AF_SECURITY ", log_string);break;
						case AF_KEY:sprintf(log_string, "%sSOCK_AF_KEY ", log_string);break;
						case AF_NETLINK:sprintf(log_string, "%sSOCK_AF_NETLINK ", log_string); break; // same as AF_ROUTE
						case AF_PACKET:sprintf(log_string, "%sSOCK_AF_PACKET ", log_string);break;
						case AF_ASH:sprintf(log_string, "%sSOCK_AF_ASH ", log_string);break;
						case AF_ECONET:sprintf(log_string, "%sSOCK_AF_ECONET ", log_string);break;
						case AF_ATMSVC:sprintf(log_string, "%sSOCK_AF_ATMSVC ", log_string);break;
						case AF_RDS:sprintf(log_string, "%sSOCK_AF_RDS ", log_string);break;
						case AF_SNA:sprintf(log_string, "%sSOCK_AF_SNA ", log_string);break;
						case AF_IRDA:sprintf(log_string, "%sSOCK_AF_IRDA ", log_string);break;
						case AF_PPPOX:sprintf(log_string, "%sSOCK_AF_PPPOX ", log_string);break;
						case AF_WANPIPE:sprintf(log_string, "%sSOCK_AF_WANPIPE ", log_string);break;
						case AF_LLC:sprintf(log_string, "%sSOCK_AF_LLC ", log_string);break;
						case AF_CAN:sprintf(log_string, "%sSOCK_AF_CAN ", log_string);break;
						case AF_TIPC:sprintf(log_string, "%sSOCK_AF_TIPC ", log_string);break;
						case AF_BLUETOOTH:sprintf(log_string, "%sSOCK_AF_BLUETOOTH ", log_string);break;
						case AF_IUCV:sprintf(log_string, "%sSOCK_AF_IUCV ", log_string);break;
						case AF_RXRPC:sprintf(log_string, "%sSOCK_AF_RXRPC ", log_string);break;
						case AF_ISDN:sprintf(log_string, "%sSOCK_AF_ISDN ", log_string);break;
						case AF_PHONET:sprintf(log_string, "%sSOCK_AF_PHONET ", log_string);break;
						case AF_IEEE802154:sprintf(log_string, "%sSOCK_AF_IEEE802154 ", log_string);break;
						case AF_MAX:sprintf(log_string, "%sSOCK_AF_MAX ", log_string);break;
						default:sprintf(log_string, "%sSOCK_NONE ", log_string);
						
					}
				}else sprintf(log_string, "%sSOCK_NONE ", log_string);
			
			}else{
				sprintf(log_string, "%sNONE ", log_string);				
				//printk(KERN_ALERT "Zonouz:: bogus i_mode (%o) for inode %s:%lu\n", f->f_dentry->d_inode->i_mode, f->f_dentry->d_inode->i_sb->s_id, f->f_dentry->d_inode->i_ino);
			}
			
			sprintf(log_string, "%s\n", log_string);
			
		}else{ 
			sprintf(log_string, "%s::Saman::ERROR:: fd=%i or f (file pointer) is NULL!\n", fd, log_string);	
		}
	}
	if (length_check != strlen(log_string))
		printk(KERN_ALERT "%s", log_string);
	return;
}



asmlinkage ssize_t (*orig_write)(int fd, const void *buf, size_t count);
asmlinkage ssize_t my_write(int fd, const void *buf, size_t count)
{	
	int ret = orig_write(fd, buf, count);
	if (ret != -1){
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " write");		
		log_fd(log_string_1, fd);
	}
	return ret;
}

asmlinkage ssize_t (*orig_read)(unsigned fd, char *buf, size_t count);
asmlinkage ssize_t my_read(unsigned int fd, char *buf, size_t count)
{
	int ret = orig_read(fd, buf, count);
	if (ret != -1){
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " read");		
		log_fd(log_string_1, fd);
	}
	return ret;
}


asmlinkage ssize_t (*orig_writev)(int fd, const struct iovec *vector, int count);
asmlinkage ssize_t my_writev(int fd, const struct iovec *vector, int count)
{
	int ret = orig_writev(fd, vector, count);
	if (ret != -1){
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " writev");
		log_fd(log_string_1, fd);
	}
	return ret;
}

asmlinkage ssize_t (*orig_readv)(int fd, const struct iovec *vector, int count);
asmlinkage ssize_t my_readv(int fd, const struct iovec *vector, int count)
{
	int ret = orig_readv(fd, vector, count);
	if (ret != -1){
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " readv");
		log_fd(log_string_1, fd);
	}
	return ret;
}


// begin
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

//if (!comment){ // this part has not been tested so is put to comment.

asmlinkage int (*orig_clone)(struct pt_regs regs);
asmlinkage int my_clone(struct pt_regs regs)
{
	int returncode = orig_clone(regs);
	if (returncode > 0){
		printk(KERN_ALERT "::PARISA:: COMMAND: %s clone PROCESS: %s PID: %i SECCON: %s OBJTYPE: PROCESS %s ", 
			WRITE_CMD, current->comm, current->pid, current_sec_context(), current->comm);
	}
	return returncode;
}

asmlinkage int (*orig_fork)(struct pt_regs regs);
asmlinkage int my_fork(struct pt_regs regs)
{
	int returncode = orig_fork(regs);
	if (returncode > 0){
		printk(KERN_ALERT "::PARISA:: COMMAND: %s fork PROCESS: %s PID: %i SECCON: %s OBJTYPE: PROCESS %s ", 
			WRITE_CMD, current->comm, current->pid, current_sec_context(), current->comm);
	}
	return returncode;
}

asmlinkage int (*orig_vfork)(struct pt_regs regs);
asmlinkage int my_vfork(struct pt_regs regs)
{
	int returncode = orig_vfork(regs);
	if (returncode > 0){
		printk(KERN_ALERT "::PARISA:: COMMAND: %s vfork PROCESS: %s PID: %i SECCON: %s OBJTYPE: PROCESS %s ", 
			WRITE_CMD, current->comm, current->pid, current_sec_context(), current->comm);
	}
	return returncode;
}


/* deal with symbols that are not exported anymore */
static int (*do_execve_fn)(char *, char **, char **, struct pt_regs *) = NULL;

/* keep this part of sys_exeve up-to-date with different versions of Linux */
#define POST_EXECVE() if (error == 0) { \
		task_lock(current); \
				current->ptrace &= ~PT_DTRACE; \
		task_unlock(current); \
		set_thread_flag(TIF_IRET); \
	} \


asmlinkage int (*orig_execve)(struct pt_regs regs);
asmlinkage int my_execve(struct pt_regs regs)
{
	int error;
	char *filename;

	filename = getname((char *)(regs.bx));
	error = PTR_ERR(filename);
	if (IS_ERR(filename))
		goto out;
	
	// logging to syslog
	//printk(KERN_ALERT "::Saman:: (%s-%i, execve, %s)", current->comm, current->pid, filename);
	printk(KERN_ALERT "::Saman:: COMMAND: %s execve PROCESS: %s PID: %i SECCON: %s OBJTYPE: PROCESS %s ", 
			WRITE_CMD, current->comm, current->pid, current_sec_context(), filename);
	
	// actually execute.
	error = (do_execve_fn)(filename, (char **)(regs.cx), (char **)(regs.dx), &regs);
	
	POST_EXECVE();
out:
	return error;
}

asmlinkage int (*orig_open)(const char *pathname, int flags, mode_t mode);
asmlinkage int my_open(const char *pathname, int flags, mode_t mode)
{
	int fd = orig_open(pathname, flags, mode);
	if (fd != -1){
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " open");
		log_fd(log_string_1, fd);
	}
	return fd;
}

asmlinkage int (*orig_creat)(const char *pathname, mode_t mode);
asmlinkage int my_creat(const char *pathname, mode_t mode)
{
	int fd = orig_creat(pathname, mode);
	if (fd != -1){
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " creat");
		log_fd(log_string_1, fd);
	}
	return fd;
}

asmlinkage long (*orig_mmap)(unsigned long addr, unsigned long len, unsigned long prot,
						unsigned long flags, unsigned long fd, off_t pgoff);
asmlinkage long my_mmap(unsigned long addr, unsigned long len, unsigned long prot,
						unsigned long flags, unsigned long fd, off_t pgoff)
{
	int ret = orig_mmap(addr, len, prot, flags, fd, pgoff);
	if (ret != -1){
		// TODO: determine if this is write or read based on fd's permissions
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " mmap");
		log_fd(log_string_1, fd);
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " mmap");
		log_fd(log_string_1, fd);
	}
	return ret;
}

asmlinkage long (*orig_mmap2)(unsigned long addr, unsigned long len, unsigned long prot,
						unsigned long flags, unsigned long fd, unsigned long pgoff);
asmlinkage long my_mmap2(unsigned long addr, unsigned long len, unsigned long prot,
						unsigned long flags, unsigned long fd, unsigned long pgoff)
{
	int ret = orig_mmap2(addr, len, prot, flags, fd, pgoff);
	if (ret != -1){
		// TODO: determine if this is write or read based on fd's permissions
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " mmap2");
		log_fd(log_string_1, fd);
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " mmap2");
		log_fd(log_string_1, fd);
	}
	
	return ret;
}


//socket-related calls
asmlinkage long (*orig_recv)(int s, void *buf, size_t len, int flags);
asmlinkage long my_recv(int s, void *buf, size_t len, int flags)
{
	int ret = orig_recv(s, buf, len, flags);
	if (ret != -1){
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " recv");
		log_fd(log_string_1, s);
	}
	
	return ret;
}

asmlinkage long (*orig_recvfrom)(int s, void *buf, size_t len, int flags, struct sockaddr *from, int *fromlen);
asmlinkage long my_recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, int *fromlen)
{
	int ret = orig_recvfrom(s, buf, len, flags, from, fromlen);
	if (ret != -1){
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " recvfrom");
		log_fd(log_string_1, s);
	}
	
	return ret;
}

asmlinkage long (*orig_recvmsg)(int s, struct msghdr *msg, int flags);
asmlinkage long my_recvmsg(int s, struct msghdr *msg, int flags)
{
	int ret = orig_recvmsg(s, msg, flags);
	if (ret != -1){
		strcpy(log_string_1, READ_CMD);
		strcat(log_string_1, " recvfrom");
		log_fd(log_string_1, s);
	}
	
	return ret;
}

asmlinkage long (*orig_send)(int s, const void *msg, size_t len, int flags);
asmlinkage long my_send(int s, const void *msg, size_t len, int flags)
{
	int ret = orig_send(s, msg, len, flags);
	if (ret != -1){
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " send");
		log_fd(log_string_1, s);
	}
	
	return ret;
}

asmlinkage long (*orig_sendto)(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, int tolen);
asmlinkage long my_sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, int tolen)
{
	int ret = orig_sendto(s, msg, len, flags, to, tolen);
	if (ret != -1){
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " sendto");
		log_fd(log_string_1, s);
	}
	
	return ret;
}

asmlinkage long (*orig_sendmsg)(int s, const struct msghdr *msg, int flags);
asmlinkage long my_sendmsg(int s, const struct msghdr *msg, int flags)
{
	int ret = orig_sendmsg(s, msg, flags);
	if (ret != -1){
		strcpy(log_string_1, WRITE_CMD);
		strcat(log_string_1, " sendmsg");
		log_fd(log_string_1, s);
	}
	
	return ret;
}

asmlinkage long (*orig_socketcall)(int call, unsigned long *args);
asmlinkage long my_socketcall(int call, unsigned long *args)
{
	unsigned long a[6];
	int ret = orig_socketcall(call, args);
	
	bool interesting = true;
	switch (call) {
		
		case SYS_SEND:
			strcpy(log_string_1, WRITE_CMD);
			strcat(log_string_1, " socketcall:send");
			break;
		case SYS_SENDTO:
			strcpy(log_string_1, WRITE_CMD);
			strcat(log_string_1, " socketcall:sendto");
			break;
		case SYS_SENDMSG:
			strcpy(log_string_1, WRITE_CMD);
			strcat(log_string_1, " socketcall:sendmsg");
			break;
		case SYS_RECV:
			strcpy(log_string_1, READ_CMD);
			strcat(log_string_1, " socketcall:recv");
			break;
		case SYS_RECVFROM:
			strcpy(log_string_1, READ_CMD);
			strcat(log_string_1, " socketcall:recvfrom");
			break;
		case SYS_RECVMSG:
			strcpy(log_string_1, READ_CMD);
			strcat(log_string_1, " socketcall:recvmsg");
			break;
		
		default:
			interesting = false;
	}
	
	// when calling copy_from_user we are only interested in the first a which is the socket fd.
	if (interesting  &&  0 <= ret  &&  !copy_from_user(a, args, 1 * sizeof(unsigned long))){
		log_fd(log_string_1, a[0]);
	}
	
	return ret;
}




// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// end


asmlinkage int (*orig_ipc)(unsigned int call, int first, int second, int third, void *ptr, long fifth);
asmlinkage int my_ipc(unsigned int call, int first, int second, int third, void *ptr, long fifth)
{	
	
	int id=-1, result;
	char command[32] = "", object_type[32] = "";
	bool log = false;
	
	call &= 0xffff;
	
	if (call <= SEMCTL){
		switch (call){
			strcpy(object_type, "IPCSEM"); // currently, we do not support semaphores!
			case SEMOP:
				//return sys_semop (first, ptr, second);
				break;
			case SEMGET:
				//return sys_semget (first, second, third);
				break;
			case SEMCTL:
				//return sys_semctl (first, second, third, fourth);
				break;
		}
	}
	if (call <= MSGCTL){
		strcpy(object_type, "IPCMSG");
		switch (call){
			case MSGSND:
				log = true;
				id = first;
				strcpy(command, WRITE_CMD);
				
				//return sys_msgsnd (first, ptr, second, third);
				break;
			case MSGRCV:
				log = true;
				id = first;
				strcpy(command, READ_CMD);			
				//return sys_msgrcv (first, ptr, second, fifth, third);
				break;
			case MSGGET:
				//return sys_msgget ((key_t) first, second);
				break;
			case MSGCTL:
				//return sys_msgctl (first, second, ptr);
				break;
		}
	}	
	if (call <= SHMCTL){
		strcpy(object_type, "IPCSHM"); // currently, we do not support shared memory!
		switch (call){
			case SHMAT:
				//return do_shmat (first, ptr, second, &raddr);
				break;
			case SHMDT:
				//return sys_shmdt (ptr);
				break;
			case SHMGET:
				//return sys_shmget (first, second, third);
				break;
			case SHMCTL:
				//return sys_shmctl (first, second, ptr);
				break;
		}
	}
	
	result = orig_ipc(call, first, second, third, ptr, fifth);
	
	//if (result != -1  &&  !filtered())
	//	printk(KERN_ALERT "::Saman:: Ipc ([%s %i]) %s", current->comm, current->pid, object_type);
		
	if (log  &&  result != -1  &&  !filtered()){
		printk(KERN_ALERT "::Saman:: COMMAND: %s ipc PROCESS: %s PID: %i SECCON: %s OBJTYPE: %s %i \n", command, current->comm, current->pid, current_sec_context(), object_type, id);
	}
	return result;
}




static void __exit my_exit( void )
{
	printk( "<1>Removing \'%s\' module\n", modname );	
	
	
	/*
	sys_call_table[ __NR_openat ] = (unsigned long)orig_openat;
	sys_call_table[ __NR_dup ] = (unsigned long)orig_dup;
	sys_call_table[ __NR_dup2 ] = (unsigned long)orig_dup2;
	sys_call_table[ __NR_close ] = (unsigned long)orig_close;
	*/
	
	sys_call_table[ __NR_write ] = (unsigned long)orig_write;
	sys_call_table[ __NR_writev ] = (unsigned long)orig_writev;
	sys_call_table[ __NR_read ] = (unsigned long)orig_read;
	sys_call_table[ __NR_readv ] = (unsigned long)orig_readv;
	sys_call_table[ __NR_ipc ] = (unsigned long)orig_ipc;
	
	if (!comment){
	
		sys_call_table[ __NR_clone ] = (unsigned long)orig_clone;
		sys_call_table[ __NR_fork ] = (unsigned long)orig_fork;
		sys_call_table[ __NR_vfork ] = (unsigned long)orig_vfork;
		sys_call_table[ __NR_execve ] = (unsigned long)orig_execve;
		//sys_call_table[ __NR_open ] = (unsigned long)orig_open;
		//sys_call_table[ __NR_creat ] = (unsigned long)orig_creat;
		sys_call_table[ __NR_mmap ] = (unsigned long)orig_mmap;
		sys_call_table[ __NR_mmap2 ] = (unsigned long)orig_mmap2;
		
		
		sys_call_table[ __NR_socketcall ] = (unsigned long)orig_socketcall;
		
		/* included in socketcall (following __NR_XXX are not in kernel anymore).
		sys_call_table[ __NR_recv ] = (unsigned long)orig_recv;
		sys_call_table[ __NR_recvfrom ] = (unsigned long)orig_recvfrom;
		sys_call_table[ __NR_recvmsg ] = (unsigned long)orig_recvmsg;
		sys_call_table[ __NR_send ] = (unsigned long)orig_send;
		sys_call_table[ __NR_sendto ] = (unsigned long)orig_sendto;
		sys_call_table[ __NR_sendmsg ] = (unsigned long)orig_sendmsg;
		*/
		
	}
		
	//pgtbl[ pindex ] = save_old_pgtable_entry;
}






/* A hack to get the address of an unexported symbol.
 * This won't work on systems that do not keep function pointers
 * in /proc/kallsyms */

#define KSYM_NAME_LEN 128

struct kallsym_iter
{
        loff_t pos;
        unsigned long value;
        unsigned int nameoff; 
        char type;
        //char name[KSYM_NAME_LEN+1];
        char name[KSYM_NAME_LEN+1];
        char module_name[MODULE_NAME_LEN + 1];
        int exported;
};

static int __init get_symbol(char *symbol, void **addr)
{
        struct file *kallsyms;
        struct seq_file *seq;
        struct kallsym_iter *iter;
        loff_t pos = 0;
        int ret = -EINVAL;
		__NR_ftruncate64;
        kallsyms = filp_open("/proc/kallsyms", O_RDONLY, 0);
        if (!kallsyms || IS_ERR(kallsyms)) {
                if (IS_ERR(kallsyms))
                        ret = PTR_ERR(kallsyms);
                printk(KERN_WARNING "PARISA: /proc/kallsyms: open: %d\n", ret);
                goto done;
        }
        seq = kallsyms->private_data;
        if (!seq) {
                printk(KERN_WARNING "PARISA: /proc/kallsyms: no private data\n");
                goto err_close;
        }
        *addr = NULL;
        for (iter = seq->op->start(seq, &pos); iter;
             iter = seq->op->next(seq, iter, &pos))
                if (!strcmp(iter->name, symbol))
                        *addr = (void *)iter->value;
        if (*addr == NULL) {
                printk(KERN_WARNING "PARISA: /proc/kallsyms: %s not found\n", symbol);
        } else {
                printk(KERN_INFO "PARISA: /proc/kallsyms: %s has address = 0x%x\n",
                       symbol, (unsigned int)*addr);
                ret = 0;
        }
 err_close:
       filp_close(kallsyms, NULL);
 done:
       return ret;
}

// Get sys_call_table on x86 machines since it is not exported from the kernel anymore 
static unsigned long *locate_sys_call_table(void)
{
	struct idt_t {
		unsigned short off1;
		unsigned short sel;
		unsigned char none,flags;
		unsigned short off2;
	} __attribute__ ((packed)) idt;
	long long idtr;
	long idtr_base;
    unsigned long *sys_call_table = 0;
	char *sys_call_asm;
	int i;

    asm ("sidt %0" : "=m" (idtr));
	idtr_base = idtr >> 16;
	// read-in IDT for 0x80 vector (syscall) 
	idt = *(struct idt_t *)(idtr_base + 8 * 0x80);
	// sys_call_asm is the assembly of the system call interrupt handler
	sys_call_asm = (char *)((idt.off2 << 16) | idt.off1);
	for (i = 0; i < 128; i++) { // look for specific assembly
		if ((sys_call_asm[i] == '\xff') &&
			(sys_call_asm[i+1] == '\x14') &&
			(sys_call_asm[i+2] == '\x85')) {
			sys_call_table = *(unsigned long **)(sys_call_asm + i + 3);
			printk(KERN_INFO "PARISA: Located sys_call_table at 0x%p\n", sys_call_table);
			break;
		}
	}
	return sys_call_table;
}


static int __init my_init( void )
{
	int level = -1;
	pte_t *(*lookup_address_fn)(unsigned long, unsigned int *) = 0;
	pte_t *pte, new_pte;

	// Locate sys_call_table in kernel 
	sys_call_table = (void **)locate_sys_call_table();
	if (!sys_call_table) {
		printk(KERN_ERR "PARISA: unable to find syscall table\n");
		return -EBUSY;
	}
	// this will not work when running two forensix modules sanity check 
	if (sys_call_table[__NR_close] != sys_close) {
		printk(KERN_ERR "PARISA: syscall table is bad\n");
		return -EBUSY;
	}
	// fix kernel permissions for two pages 
	if (get_symbol("lookup_address", (void **)&lookup_address_fn) < 0) {
		return -EINVAL;
	}
	pte = (lookup_address_fn)((unsigned long)sys_call_table, &level);
	if (!pte) {
		printk(KERN_ERR "PARISA: lookup_address didn't work\n");
		return -EINVAL;
	}
	printk("PARISA: pte = 0x%lx\n", (unsigned long)pte->pte);
	new_pte = *pte;
	new_pte.pte |= _PAGE_RW;
	set_pte_atomic(pte, new_pte);
	__flush_tlb_all();
	pte = (lookup_address_fn)((unsigned long)sys_call_table, &level);
	if (!pte) {
		printk(KERN_ERR "PARISA: lookup_address didn't work\n");
		return -EINVAL;
	}
	printk("PARISA: new pte = 0x%lx\n", (unsigned long)pte->pte);
	
	// test if we can write to the sys_call_table 
	sys_call_table[__NR_close] = sys_close;
	
	if (get_symbol("do_execve", (void **)&do_execve_fn) < 0)
		return -EINVAL;
		
	
	/* Remember to uncomment in __exit(): pgtbl[ pindex ] = save_old_pgtable_entry;
	// This code is also working to make the syscall_table writable
	// but it requires to get syscall_table address from /boot/System
	sys_call_table=(unsigned long *)0xc0592150;
	
	printk( "<1>\nInstalling \'%s\' module ", modname );
	printk( "(sys_call_table[] at %08X) \n", (int)sys_call_table );

	// get current values from control-registers CR3 and CR4
	asm(" mov %%cr4, %%eax \n mov %%eax, _cr4 " ::: "ax" );
	asm(" mov %%cr3, %%eax \n mov %%eax, _cr3 " ::: "ax" );

	// confirm that processor is using the legacy paging mechanism
	if ( (_cr4 >> 5) & 1 ){ 
		printk( " processor is using Page-Address Extensions \n");
		return 	-ENOSYS;
	} 
	
	// extract paging-table indices from 'sys_call_table[]' address 
	dindex = ((int)sys_call_table >> 22) & 0x3FF;	// pgdir-index
	pindex = ((int)sys_call_table >> 12) & 0x3FF;	// pgtbl-index

	// setup pointers to the page-directory and page-table frames
	pgdir = phys_to_virt( _cr3 & ~0xFFF );
	pgtbl = phys_to_virt( pgdir[ dindex ] & ~0xFFF );	

	// preserve page-table entry for the 'sys_call_table[]' frame
	save_old_pgtable_entry = pgtbl[ pindex ];	

	printk( "\nInstalling new function for system-call %d\n", __NR_break );
	pgtbl[ pindex ] |= 2;	// make sure that page-frame is 'writable'
	
	*/
	
	/*	

	orig_openat = (void *)sys_call_table[ __NR_openat ];
	sys_call_table[ __NR_openat ] = (unsigned long)my_openat;
	
	orig_dup = (void *)sys_call_table[ __NR_dup ];
	sys_call_table[ __NR_dup ] = (unsigned long)my_dup;
	
	orig_dup2 = (void *)sys_call_table[ __NR_dup2 ];
	sys_call_table[ __NR_dup2 ] = (unsigned long)my_dup2;
	
	orig_close = (void *)sys_call_table[ __NR_close ];
	sys_call_table[ __NR_close ] = (unsigned long)my_close;
	*/
	
	orig_write = (void *)sys_call_table[ __NR_write ];
	sys_call_table[ __NR_write ] = (unsigned long)my_write;
	
	orig_writev = (void *)sys_call_table[ __NR_writev ];
	sys_call_table[ __NR_writev ] = (unsigned long)my_writev;
	
	orig_read = (void *)sys_call_table[ __NR_read ];
	sys_call_table[ __NR_read ] = (unsigned long)my_read;
	
	orig_readv = (void *)sys_call_table[ __NR_readv ];
	sys_call_table[ __NR_readv ] = (unsigned long)my_readv;	
	
	orig_ipc = (void *)sys_call_table[ __NR_ipc ];
	sys_call_table[ __NR_ipc ] = (unsigned long)my_ipc;
	
	if (!comment){
		
		orig_clone = (void *)sys_call_table[ __NR_clone ];
		sys_call_table[ __NR_clone ] = (unsigned long)my_clone;
			
		orig_fork = (void *)sys_call_table[ __NR_fork ];
		sys_call_table[ __NR_fork ] = (unsigned long)my_fork;
			
		orig_vfork = (void *)sys_call_table[ __NR_vfork ];
		sys_call_table[ __NR_vfork ] = (unsigned long)my_vfork;
		
		orig_execve = (void *)sys_call_table[ __NR_execve ];
		sys_call_table[ __NR_execve ] = (unsigned long)my_execve;
		
		// is working just is not needed (not explicit write or read).
		//orig_open = (void *)sys_call_table[ __NR_open ];
		//sys_call_table[ __NR_open ] = (unsigned long)my_open;
		
		//orig_creat = (void *)sys_call_table[ __NR_creat ];
		//sys_call_table[ __NR_creat ] = (unsigned long)my_creat;
		
		orig_mmap = (void *)sys_call_table[ __NR_mmap ];
		sys_call_table[ __NR_mmap ] = (unsigned long)my_mmap;

		orig_mmap2 = (void *)sys_call_table[ __NR_mmap2 ];
		sys_call_table[ __NR_mmap2 ] = (unsigned long)my_mmap2;

		orig_socketcall = (void *)sys_call_table[ __NR_socketcall ];
		sys_call_table[ __NR_socketcall ] = (unsigned long)my_socketcall;
		
		/*
		orig_recv = (void *)sys_call_table[ __NR_recv ];
		sys_call_table[ __NR_recv ] = (unsigned long)my_recv;
		
		orig_recvfrom = (void *)sys_call_table[ __NR_recvfrom ];
		sys_call_table[ __NR_recvfrom ] = (unsigned long)my_recvfrom;
		
		orig_recvmsg = (void *)sys_call_table[ __NR_recvmsg ];
		sys_call_table[ __NR_recvmsg ] = (unsigned long)my_recvmsg;
		
		orig_send = (void *)sys_call_table[ __NR_send ];
		sys_call_table[ __NR_send ] = (unsigned long)my_send;
		
		orig_sendto = (void *)sys_call_table[ __NR_sendto ];
		sys_call_table[ __NR_sendto ] = (unsigned long)my_sendto;
		
		orig_sendmsg = (void *)sys_call_table[ __NR_sendmsg ];
		sys_call_table[ __NR_sendmsg ] = (unsigned long)my_sendmsg;
		*/
		
	}	
		
	return	0;  // SUCCESS
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");

















































/*


asmlinkage int (*orig_openat)(const char * pathname, int flags, ...);
asmlinkage int my_openat(const char * pathname, int flags, mode_t mode)
{
	int result;
	//mode_t mode;
	
	//va_list argp;
	//va_start(argp, flags);
	
	if (flags & O_CREAT){
		//mode = (mode_t) va_arg(argp, long);
		printk(KERN_ALERT "3 taayi!");
		result = orig_openat(pathname, flags, mode);
	}else{
		printk(KERN_ALERT "2 taayi!");
		result = orig_openat(pathname, flags);
	}
	
	//va_end(argp);
	
	//int result = orig_open(pathname, flags, mode);
	if (result != -1  &&  !filtered()){
		printk(KERN_ALERT "::Saman:: Openat ([%s %i], %s) = %i", current->comm, current->pid, pathname, result);
	}
	return result;
}

asmlinkage int (*orig_dup)(int oldfd);
asmlinkage int my_dup(int oldfd)
{
	int result = orig_dup(oldfd);
	if (result != -1  &&  !filtered())
		printk(KERN_ALERT "::Saman:: Dup ([%s %i], %i) = %i", current->comm, current->pid, oldfd, result);
	return result;
}

asmlinkage int (*orig_dup2)(int oldfd, int newfd);
asmlinkage int my_dup2(int oldfd, int newfd)
{
	int result = orig_dup2(oldfd, newfd);
	if (result != -1  &&  !filtered())
		printk(KERN_ALERT "::Saman:: Dup2 ([%s %i], %i) = %i", current->comm, current->pid, oldfd, result);
	return result;
}

asmlinkage int (*orig_creat)(const char * pathname, mode_t mode);
asmlinkage int my_creat(const char * pathname, mode_t mode)
{
	int result = orig_creat(pathname, mode);
	if (result != -1  &&  !filtered())
		printk(KERN_ALERT "::Saman:: Creat ([%s %i], %s) = %i", current->comm, current->pid, pathname, result);
	return result;
}

asmlinkage int (*orig_pipe)(int filedes[2]);
asmlinkage int my_pipe(int filedes[2])
{
	int result = orig_pipe(filedes);
	if (result != -1  &&  !filtered())
		printk(KERN_ALERT "::Saman:: Pipe ([%s %i]) (%i -> %i)", current->comm, current->pid, filedes[0], filedes[1]);
	return result;
}

asmlinkage int (*orig_close)(int fd);
asmlinkage int my_close(int fd)
{
	int result = orig_close(fd);
	if (result != -1  &&  !filtered())
		printk(KERN_ALERT "::Saman:: Close ([%s %i], %i)", current->comm, current->pid, fd);
	return result;
}






*/
