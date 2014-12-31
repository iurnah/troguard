#include <iostream>

int main(int argc, char** argv){
	char buf[30];
	snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
	FILE* pf = fopen(buf, "r");
	if (pf) {
		unsigned size; //       total program size
		unsigned resident;//   resident set size
		unsigned share;//      shared pages
		unsigned text;//       text (code)
		unsigned lib;//        library
		unsigned data;//       data/stack
		unsigned dt;//         dirty pages (unused in Linux 2.6)
		fscanf(pf, "%u" /* %u %u %u %u %u"*/, &size/*, &resident, &share, &text, &lib, &data*/);
		DOMSGCAT(MSTATS, std::setprecision(4) << size / (1024.0) << "MB mem used");
	}
	fclose(pf);
}
