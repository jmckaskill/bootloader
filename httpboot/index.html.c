#ifdef __arm__
#define ALIGNED __attribute((aligned(4)))
#endif
extern unsigned long g_httpboot_index_html_sz;
extern const char g_httpboot_index_html[];
unsigned long g_httpboot_index_html_sz = 125;
const char ALIGNED g_httpboot_index_html[] = ""
"\x48\x54\x54\x50\x2f\x31\x2e\x31\x20\x4f\x4b\x0d\x0a\x43\x6f\x6e"/* HTTP/1.1 OK..Con */
"\x74\x65\x6e\x74\x2d\x54\x79\x70\x65\x3a\x74\x65\x78\x74\x2f\x68"/* tent-Type:text/h */
"\x74\x6d\x6c\x0d\x0a\x43\x6f\x6e\x74\x65\x6e\x74\x2d\x45\x6e\x63"/* tml..Content-Enc */
"\x6f\x64\x69\x6e\x67\x3a\x67\x7a\x69\x70\x0d\x0a\x43\x6f\x6e\x74"/* oding:gzip..Cont */
"\x65\x6e\x74\x2d\x4c\x65\x6e\x67\x74\x68\x3a\x34\x34\x0d\x0a\x0d"/* ent-Length:44... */
"\x0a"/* . */
"\x1f\x8b\x08\x00\x00\x00\x00\x00\x02\x0a\xb3\xc9\x28\xc9\xcd\xb1"/* ............(... */
"\xe3\xf2\x48\xcd\xc9\xc9\x57\x08\xcf\x2f\xca\x49\x51\xe4\xb2\xd1"/* ..H...W../.IQ... */
"\x07\x0b\x02\x00\x75\xba\x91\x45\x1b\x00\x00\x00"/* ....u..E.... */;

