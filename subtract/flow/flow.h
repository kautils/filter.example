#ifndef FLOW_FILTER_ARITHMETIC_SUBTRACT_SUBTRACT_FLOW_FLOW_CC
#define FLOW_FILTER_ARITHMETIC_SUBTRACT_SUBTRACT_FLOW_FLOW_CC

#include <stdint.h>

#define FILTER_DATABASE_OPTION_OVERWRITE 1 // ignore / overwrite
#define FILTER_DATABASE_OPTION_WITHOUT_ROWID 2 // not use rowid, if database dose not support rowid, then shoudl be ignored (e.g : in case of using filesystem directlly).
#define FILTER_DATABASE_TYPE_SQLITE3 1

struct filter_handler;
struct filter_database_handler;
struct filter_input_static;
struct filter;
struct filter_lookup_table;
struct filter_lookup_elem;

filter_handler* filter_handler_initialize();
void filter_handler_free(filter_handler * fhdl);
//int filter_handler_link(filter_handler * fhdl);
int filter_handler_push(filter_handler * fhdl,filter* f);
void filter_handler_set_io_length(filter_handler * hdl,uint64_t);
void filter_handler_set_local_uri(filter_handler * hdl,const char * uri);
int filter_database_setup(filter * f);
int filter_database_save(filter * f);

void* filter_input(filter * f);
uint64_t filter_input_bytes(filter * f);
void * filter_lookup(filter_lookup_table * flookup_table,const char * key);


struct filter_lookup_table{};
//struct filter_input_static{ void * data=0;uint64_t bytes=0; };
struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(8)));

struct filter{
    int (*main)(filter * m)=0;
    void * (*output)(filter *)=0;
    uint64_t(*output_bytes)(filter *)=0;
    void * (*input)(filter *)=filter_input;
    uint64_t(*input_bytes)(filter *)=filter_input_bytes;
    const char *(*id)(filter *)=0;
    const char *(*id_hr)(filter *)=0;
    int (*database_type)(filter *)=0;
    int (*setup_database)(filter *)=filter_database_setup;
    int (*save)(filter *)=filter_database_save;
    void* m=0;
//    filter_input_static input_static; // short cut to filter
    filter_database_handler * db=0;
    filter_handler * hdl=0;
    int option=0;
    int pos=-1;
} __attribute__((aligned(8)));



filter_database_handler* filter_database_handler_initialize(filter * f);
void filter_database_handler_free(filter_database_handler * hdl);
int filter_database_handler_reset(filter_database_handler * hdl);
int filter_database_handler_set_uri(filter_database_handler * hdl,const char * prfx,const char * id);
int filter_database_handler_setup(filter_database_handler * hdl);
int filter_database_handler_set_output(filter_database_handler * hdl,const void * begin,const void * end);
int filter_database_handler_set_input(filter_database_handler * hdl,const void * begin,const void * end);
int filter_database_handler_set_io_length(filter_database_handler * hdl,uint64_t);
int filter_database_handler_save(filter_database_handler * hdl);
int filter_database_handler_set_option(filter_database_handler * hdl,int op);


struct filter_database_handler{
    int (*set_uri)(filter_database_handler * hdl,const char * prfx,const char * id)=filter_database_handler_set_uri;
    filter_database_handler* (*alloc)(filter * f)=filter_database_handler_initialize;
    void (*free)(filter_database_handler * hdl)=filter_database_handler_free;
    int (*setup)(filter_database_handler * hdl)=filter_database_handler_setup;
    int (*reset)(filter_database_handler * hdl)=filter_database_handler_reset;
    int (*set_output)(filter_database_handler * hdl,const void * begin,const void * end)=filter_database_handler_set_output;
    int (*set_input)(filter_database_handler * hdl,const void * begin,const void * end)=filter_database_handler_set_input;
    int (*set_io_length)(filter_database_handler * hdl,uint64_t)=filter_database_handler_set_io_length;
    int (*save)(filter_database_handler * hdl)=filter_database_handler_save;
    int (*set_option)(filter_database_handler * hdl,int op)=filter_database_handler_set_option;
    int type=0;
    void * instance=0;
    int last_option=-1;
};






#endif