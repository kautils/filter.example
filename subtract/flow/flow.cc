#include "./flow.h"
#include "stdint.h"
#include <vector>
#include <string>

#include "kautil/sqlite3/sqlite3.h"
#include "libgen.h"
#include "sys/stat.h"


struct io_data{ const void * begin;const void * end; };
struct filter_database_sqlite3_handler{
    kautil::database::Sqlite3Stmt * create;
    kautil::database::Sqlite3Stmt * insert;
    kautil::database::Sqlite3 * sql=0;
    kautil::database::sqlite3::Options * op=0;
    std::string uri_prfx;
    std::string id;
    
    io_data i;
    io_data o;
    uint64_t io_len=0;
    bool is_overwrite=false;
    bool is_without_rowid=false;
};

void* filter_database_sqlite_initialize();
int filter_database_sqlite_set_uri(void * whdl,const char * prfx,const char * id);
int filter_database_sqlite_reset(void* whdl);
void filter_database_sqlite_free(void* whdl);
int filter_database_sqlite_setup(void * whdl);
int filter_database_sqlite_set_output(void * whdl,const void * begin,const void * end);
int filter_database_sqlite_set_input(void * whdl,const void * begin,const void * end);
int filter_database_sqlite_set_io_length(void * whdl,uint64_t len);
int filter_database_sqlite_save(void * whdl);
//the reason why i don't use bitwise is i want to hide definitions of filter_database_handler including macro  
int filter_database_sqlite_overwrite_sw(void * whdl,bool sw); 
int filter_database_sqlite_without_rowid_sw(void * whdl,bool sw); 





///@note input is common
void* filter_input(filter * f);
uint64_t filter_input_bytes(filter * f);
int filter_database_reset(filter * f);


struct filter_handler{ 
    filter * previous=0;
    filter * current=0;
    std::vector<filter*> filters;
    std::string local_uri;
    uint64_t io_len=0;
};

struct filter_database_handler;
filter_database_handler* filter_database_handler_initialize(filter * f);

filter_handler* filter_handler_initialize(){ return new filter_handler{}; }
void filter_handler_free(filter_handler * fhdl){ delete fhdl; }
void filter_handler_set_io_length(filter_handler * hdl,uint64_t len){ hdl->io_len=len; }
void filter_handler_set_local_uri(filter_handler * hdl,const char * uri){ hdl->local_uri = uri; }


void filter_database_handler_free(filter_database_handler * hdl);
int filter_database_handler_reset(filter_database_handler * hdl);
int filter_database_handler_set_uri(filter_database_handler * hdl,const char * prfx,const char * id);
int filter_database_handler_setup(filter_database_handler * hdl);
int filter_database_handler_set_output(filter_database_handler * hdl,const void * begin,const void * end);
int filter_database_handler_set_input(filter_database_handler * hdl,const void * begin,const void * end);
int filter_database_handler_set_io_length(filter_database_handler * hdl,uint64_t);
int filter_database_handler_save(filter_database_handler * hdl);
int filter_database_handler_set_option(filter_database_handler * hdl,int op);



//struct filter_database_handler{
//    int (*set_uri)(filter_database_handler * hdl,const char * prfx,const char * id)=filter_database_handler_set_uri;
//    filter_database_handler* (*alloc)(filter * f)=filter_database_handler_initialize;
//    void (*free)(filter_database_handler * hdl)=filter_database_handler_free;
//    int (*setup)(filter_database_handler * hdl)=filter_database_handler_setup;
//    int (*reset)(filter_database_handler * hdl)=filter_database_handler_reset;
//    int (*set_output)(filter_database_handler * hdl,const void * begin,const void * end)=filter_database_handler_set_output;
//    int (*set_input)(filter_database_handler * hdl,const void * begin,const void * end)=filter_database_handler_set_input;
//    int (*set_io_length)(filter_database_handler * hdl,uint64_t)=filter_database_handler_set_io_length;
//    int (*save)(filter_database_handler * hdl)=filter_database_handler_save;
//    int (*set_option)(filter_database_handler * hdl,int op)=filter_database_handler_set_option;
//    int type=0;
//    void * instance=0;
//    int last_option=-1;
//};
//



filter_database_handler* filter_database_handler_initialize(filter * f){
    if(f->database_type){
        auto res = new filter_database_handler{};
        switch(f->database_type(f)){
            case FILTER_DATABASE_TYPE_SQLITE3:{
                res->type = f->database_type(f);
                res->instance = filter_database_sqlite_initialize();
                return res;
            };
        }
    }// database_type
    return nullptr;
}



int filter_database_reset(filter * f){
    
    auto initialized = false;
    if(!f->db) f->db = filter_database_handler_initialize(f); 
    else{
        initialized=(f->option == f->db->last_option); 
        if(!f->db->reset){  // (0) not initialized, (1) option is changed (2) reset is not defiend
            f->db->free(f->db);
            f->db=f->db->alloc(f);
        }
    }
    
    f->db->set_option(f->db,f->option); // assume want to overwrite paticular range
    f->db->set_io_length(f->db, f->hdl->io_len);
    f->db->set_uri(f->db, f->hdl->local_uri.data(), f->id_hr(f));

//    auto out = (const char *) f->output(f);
//    f->db->set_output(f->db, out, out + f->output_bytes(f));
//
//    auto in = (const char *) f->input(f);
//    f->db->set_input(f->db, in, in + f->input_bytes(f));
    
    if(!initialized | bool(!f->db->reset)){  // (0) not initialized, (1) option is changed (2) reset is not defiend
        f->db->last_option = f->option;
        return f->db->setup(f->db); 
    }
    if(f->db->reset) return f->db->reset(f->db);
//    }
    return 0;
}

int filter_database_setup(filter * f) { return filter_database_reset(f); }


void filter_database_handler_free(filter * f){
    if(f->db) {
        if (f->database_type) {
            switch (f->database_type(f)){
                case FILTER_DATABASE_TYPE_SQLITE3:filter_database_sqlite_free(f->db->instance);
            };
            delete f->db;
        } // database_type
    }// db
    f->db=nullptr;
}



//int filter_handler_link(filter_handler * fhdl){
//    for(auto & f : fhdl->filters) {
//        if(f->pos>0){
//            auto f_i = f->hdl->filters[f->pos-1];
//            f->input_static.data = f_i->output(f_i);
//            f->input_static.bytes = f_i->output_bytes(f_i);
//        }else{
//            f->input_static.data = nullptr;
//            f->input_static.bytes = 0;
//        }
//    }
//    return 0;
//}

int filter_handler_push(filter_handler * fhdl,filter* f){
    if(0==(f->pos=fhdl->filters.size())){
        fhdl->previous= nullptr;
        fhdl->current= f;
    }
    fhdl->filters.push_back(f);
    return 0;
}

void* filter_input(filter * f) { 
    if(f->pos>0){
        auto f_i = f->hdl->filters[f->pos-1];
        return f_i->output(f_i);
    }
    return nullptr; 
}

uint64_t filter_input_bytes(filter * f) { 
    if(f->pos>0){
        auto f_i = f->hdl->filters[f->pos-1];
        return f_i->output_bytes(f_i);
    }
    return 0;
}
int filter_database_save(filter * f){
    if(0== !f->db + !f->save){
        
        auto out = (const char *) f->output(f);
        f->db->set_output(f->db, out, out + f->output_bytes(f));
    
        auto in = (const char *) f->input(f);
        f->db->set_input(f->db, in, in + f->input_bytes(f));
        
        return f->db->save(f->db);
    } 
    return 0;
}



int filter_database_handler_set_option(filter_database_handler * hdl,int op){
    auto op_ow = op & FILTER_DATABASE_OPTION_OVERWRITE;
    auto op_norowid = op & FILTER_DATABASE_OPTION_WITHOUT_ROWID;
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: {
            int res= 
                  filter_database_sqlite_overwrite_sw(hdl->instance,op_ow)
                | filter_database_sqlite_without_rowid_sw(hdl->instance,op_norowid);
        }
    }
    return 1;    
}


int filter_database_handler_reset(filter_database_handler * hdl){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return  filter_database_sqlite_reset(hdl->instance);
    }
    return 1;
}
int filter_database_handler_set_uri(filter_database_handler * hdl,const char * prfx,const char * id){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return  filter_database_sqlite_set_uri(hdl->instance,prfx,id);
    }
    return 1;
}
void filter_database_handler_free(filter_database_handler * hdl){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: filter_database_sqlite_free(hdl->instance); break;
    }
}
int filter_database_handler_setup(filter_database_handler * hdl){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return filter_database_sqlite_setup(hdl->instance); break;
    }
    return 1;
}
int filter_database_handler_set_output(filter_database_handler * hdl,const void * begin,const void * end){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return filter_database_sqlite_set_output(hdl->instance,begin,end); break;
    }
    return 1;
}
int filter_database_handler_set_input(filter_database_handler * hdl,const void * begin,const void * end){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return filter_database_sqlite_set_input(hdl->instance,begin,end); break;
    }
    return 1;
}
int filter_database_handler_set_io_length(filter_database_handler * hdl,uint64_t len){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return filter_database_sqlite_set_io_length(hdl->instance,len); break;
    }
    return 1;
}
int filter_database_handler_save(filter_database_handler * hdl){
    switch(hdl->type){
        case FILTER_DATABASE_TYPE_SQLITE3: return  filter_database_sqlite_save(hdl->instance); break;
    }
    return 1;
}



void * filter_lookup(filter_lookup_table * flookup_table,const char * key){
    // sorting is bad for this process. i neither want  not to change the order of the flookup_table nor to copy it 
    auto arr = reinterpret_cast<char **>(flookup_table);
    for(;;arr+=sizeof(filter_lookup_elem)/sizeof(uintptr_t)){
        if(nullptr==arr[0]) break;
        if(!strcmp(key,arr[0]))return reinterpret_cast<void*>(arr[1]);
    }
    return nullptr;
}


constexpr static const char * kCreateSt             = "create table if not exists m([rowid] interger primary key,[k] blob,[v] blob,unique([k])) ";
constexpr static const char * kCreateStWithoutRowid = "create table if not exists m([k] blob primary key,[v] blob) without rowid ";
static const char * kInsertSt = "insert or ignore into m(k,v) values(?,?)";
static const char * kInsertStOw = "insert or replace into m(k,v) values(?,?)";





filter_database_sqlite3_handler* get_instance(void * whdl){
    return reinterpret_cast<filter_database_sqlite3_handler*>(whdl);
}

void* filter_database_sqlite_initialize(){
    auto res = new filter_database_sqlite3_handler;
    res->op = kautil::database::sqlite3::sqlite_options(); 
    return res;
}

int filter_database_sqlite_reset(void* whdl){ return 0; }

int filter_database_sqlite_set_uri(void * whdl,const char * prfx,const char * id){
    auto m=get_instance(whdl);
    m->uri_prfx = prfx; 
    m->id = id; 
    return 0;
}

void filter_database_sqlite_free(void* whdl){
    auto m=get_instance(whdl);
    delete m->op;
    delete m->sql;
    delete m;
}

int filter_database_sqlite_setup(void * whdl){
    auto m=get_instance(whdl);
    if(!m->sql) {
        auto path = m->uri_prfx +"/" + m->id + ".sqlite";
        {
            struct stat st;
            auto buf = std::string(path.data());
            auto dir = dirname(buf.data());
            if(stat(dir,&st)){
                if(mkdir(dir)){
                    printf("fail");
                    return 1;
                }
            } 
        }
        m->op = kautil::database::sqlite3::sqlite_options(); 
        m->sql = new kautil::database::Sqlite3{path.data(),m->op->sqlite_open_create()|m->op->sqlite_open_readwrite()|m->op->sqlite_open_nomutex()};
        m->create = m->sql->compile(m->is_without_rowid ? kCreateStWithoutRowid : kCreateSt);
        if(m->create){
            auto res_crt = m->create->step(true);
            res_crt |= ((m->create->step(true) == m->op->sqlite_ok()));
            if(res_crt){
                return !bool(m->insert = m->sql->compile(m->is_overwrite ? kInsertStOw : kInsertSt));
            }
        }
    }
    m->sql->error_msg();
    delete m->op;
    delete m->sql;
    m->op=nullptr;
    m->sql=nullptr;
    
    return 1;
}


int filter_database_sqlite_set_output(void * whdl,const void * begin,const void * end){
    auto m=get_instance(whdl);
    m->o.begin = begin;
    m->o.end = end;
    return m->o.begin < m->o.end; 
}

int filter_database_sqlite_set_input(void * whdl,const void * begin,const void * end){
    auto m=get_instance(whdl);
    m->i.begin = begin;
    m->i.end = end;
    return m->i.begin < m->i.end; 
}

int filter_database_sqlite_set_io_length(void * whdl,uint64_t len){
    auto m=get_instance(whdl);
    m->io_len= len;
    return 0;
}

int filter_database_sqlite_overwrite_sw(void * whdl,bool sw){
    auto m=get_instance(whdl);
    m->is_overwrite=sw;
    return 0;
}

int filter_database_sqlite_without_rowid_sw(void * whdl,bool sw){
    auto m=get_instance(whdl);
    m->is_without_rowid=sw;
    return 0;
}

int filter_database_sqlite_save(void * whdl){
    auto m=get_instance(whdl);
    if(auto begin_i = reinterpret_cast<const char*>(m->i.begin)){
        auto end_i = reinterpret_cast<const char*>(m->i.end);
        auto block_i = (end_i - begin_i) / m->io_len;
        if(auto begin_o = reinterpret_cast<const char*>(m->o.begin)){
            auto end_o = reinterpret_cast<const char*>(m->o.end);
            auto block_o = (end_o - begin_o) / m->io_len;
            auto fail = false;
            if(begin_i < begin_o){
                m->sql->begin_transaction();
                for(;begin_i != end_i; begin_i+=block_i,begin_o+=block_o){
                    auto res_stmt = !m->insert->set_blob(1,begin_i,block_i);
                    res_stmt |= !m->insert->set_blob(2,begin_o,block_o);
                    
                    auto res_step = m->insert->step(true);
                    res_step |= res_step == m->op->sqlite_ok();
                    
                    if((fail=res_stmt+!res_step))break;
                }
                if(fail) m->sql->roll_back();
                m->sql->end_transaction();
                return !fail;
            }
        }
    }else m->sql->error_msg();
    return 1;
}


