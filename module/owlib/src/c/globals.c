/*
$Id$
    OWFS -- One-Wire filesystem
    OWHTTPD -- One-Wire Web Server
    Written 2003 Paul H Alfille
    email: palfille@earthlink.net
    Released under the GPL
    See the header file: ow.h for full attribution
    1wire/iButton system from Dallas Semiconductor
*/

#include "owfs_config.h"
#include "ow.h"
#include "ow_devices.h"

speed_t speed = B9600;
int portnum = -1 ; /* TCP port (for owhttpd) */
char * portname = NULL ;
char * devport = NULL ; /* Device name (COM or parallel port) */
int devfd = -1; /*file descriptor for serial/parallel port*/
int useusb = 0 ; /* which DS9490 to connect */
/* Globals for DS2480B state */
int UMode ;
int ULevel ;
int USpeed ;
int ProgramAvailable ;
enum adapter_type Adapter ;
const char * adapter_name = "Unknown" ;
pid_t pid_num ;


#ifdef OW_USB
    usb_dev_handle * devusb = NULL ;
#else /* OW_USB */
    void * devusb = NULL ;
#endif /* OW_USB */

int presencecheck = 1 ; /* check if present whenever opening a directory or static file */
time_t start_time ;
time_t dir_time ; /* time of last directory scan */

int background = 1 ; /* operate in background mode */
int readonly = 0 ; /* readonly file system */

struct interface_routines iroutines ;

enum temp_type tempscale = temp_celsius ; /* Default Celsius */

/* Static buffer for serial conmmunications */
/* Since only used during actual transfer to/from the adapter,
   should be protected from contention even when multithreading allowed */

unsigned char combuffer[MAX_FIFO_SIZE] ;

/* ----------------- */
/* -- Statistics --- */
/* ----------------- */
unsigned int cache_flips = 0 ;
unsigned int cache_adds = 0 ;
struct average old_avg = {0L,0L,0L,0L,} ;
struct average new_avg = {0L,0L,0L,0L,} ;
struct average store_avg = {0L,0L,0L,0L,} ;
struct cache cache_ext = {0L,0L,0L,0L,0L,} ;
struct cache cache_int = {0L,0L,0L,0L,0L,} ;
struct cache cache_dir = {0L,0L,0L,0L,0L,} ;
struct cache cache_sto = {0L,0L,0L,0L,0L,} ;

unsigned int read_calls = 0 ;
unsigned int read_cache = 0 ;
unsigned int read_bytes = 0 ;
unsigned int read_cachebytes = 0 ;
unsigned int read_array = 0 ;
unsigned int read_tries[3] = {0,0,0,} ;
unsigned int read_success = 0 ;
struct average read_avg = {0L,0L,0L,0L,} ;

unsigned int write_calls = 0 ;
unsigned int write_bytes = 0 ;
unsigned int write_array = 0 ;
unsigned int write_tries[3] = {0,0,0,} ;
unsigned int write_success = 0 ;
struct average write_avg = {0L,0L,0L,0L,} ;

struct directory dir_main = { 0L, 0L, } ;
struct directory dir_dev = { 0L, 0L, } ;
unsigned int dir_depth = 0 ;
struct average dir_avg = {0L,0L,0L,0L,} ;

struct timeval bus_time = {0, 0, } ;
struct timeval bus_pause = {0, 0, } ;
unsigned int bus_locks = 0 ;
unsigned int bus_unlocks = 0 ;
unsigned int crc8_tries = 0 ;
unsigned int crc8_errors = 0 ;
unsigned int crc16_tries = 0 ;
unsigned int crc16_errors = 0 ;
unsigned int read_timeout = 0 ;

struct average all_avg = {0L,0L,0L,0L,} ;

