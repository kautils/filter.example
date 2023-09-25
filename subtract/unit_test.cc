
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <stdio.h>
#include <cstring>
#include <vector>
#include <string>
#include "kautil/cache/virtual_file/virtual_file.h"
#include "kautil/sqlite3/sqlite3.h"
#include "sys/stat.h"
#include "libgen.h"

struct filter;
struct filter_handler{ 
    filter * previous=0;
    filter * current=0;
    std::vector<filter*> filters;
    std::string local_uri;
    uint64_t io_len=0;
};

struct filter_database_handler;
void filter_database_handler_free(filter_database_handler * hdl);
int filter_database_handler_set_uri(filter_database_handler * hdl,const char * prfx,const char * id);
int filter_database_handler_setup(filter_database_handler * hdl);
int filter_database_handler_set_output(filter_database_handler * hdl,const void * begin,const void * end);
int filter_database_handler_set_input(filter_database_handler * hdl,const void * begin,const void * end);
int filter_database_handler_set_io_length(filter_database_handler * hdl,uint64_t);
int filter_database_handler_save(filter_database_handler * hdl);



void* filter_database_sqlite_initialize();
int filter_database_sqlite_set_uri(void * whdl,const char * prfx,const char * id);
void filter_database_sqlite_free(void* whdl);
int filter_database_sqlite_setup(void * whdl);
int filter_database_sqlite_set_output(void * whdl,const void * begin,const void * end);
int filter_database_sqlite_set_input(void * whdl,const void * begin,const void * end);
int filter_database_sqlite_set_io_length(void * whdl,uint64_t len);
int filter_database_sqlite_save(void * whdl);


#define FILTER_DATABASE_TYPE_SQLITE3 1
struct filter_database_handler{
    int (*set_uri)(filter_database_handler * hdl,const char * prfx,const char * id)=filter_database_handler_set_uri;
    void (*free)(filter_database_handler * hdl)=filter_database_handler_free;
    int (*setup)(filter_database_handler * hdl)=filter_database_handler_setup;
    int (*set_output)(filter_database_handler * hdl,const void * begin,const void * end)=filter_database_handler_set_output;
    int (*set_input)(filter_database_handler * hdl,const void * begin,const void * end)=filter_database_handler_set_input;
    int (*set_io_length)(filter_database_handler * hdl,uint64_t)=filter_database_handler_set_io_length;
    int (*save)(filter_database_handler * hdl)=filter_database_handler_save;
    int type=0;
    void * instance=0;
};


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



void* filter_input(filter * f);
uint64_t filter_input_bytes(filter * f);
struct filter{
    int (*main)(filter * m)=0;
    void * (*output)(filter *)=0;
    uint64_t(*output_bytes)(filter *)=0;
    void * (*input)(filter *)=filter_input;
    uint64_t(*input_bytes)(filter *)=filter_input_bytes;
    const char *(*id)(filter *)=0;
    const char *(*id_hr)(filter *)=0;
    int (*database_type)(filter *)=0;
    void* m=0;
    filter_database_handler * db=0;
    filter_handler * hdl=0;
} __attribute__((aligned(8)));
///@note input is common
void* filter_input(filter * f) { return f->hdl->previous->output(f->hdl->previous); }
uint64_t filter_input_bytes(filter * f) { 
    if(f->hdl->previous)return f->hdl->previous->output_bytes(f->hdl->previous); 
    return 0;
}




static const char * kCreateSt = "create table if not exists m([rowid] primary key,[k] blob,[v] blob,unique([k])) ";
static const char * kInsertSt = "insert or ignore into m(k,v) values(?,?)";


struct io_data{
    const void * begin;
    const void * end;
    uint64_t io_length;
};

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
};


filter_database_sqlite3_handler* get_instance(void * whdl){
//    return reinterpret_cast<filter_database_sqlite3_handler*>(reinterpret_cast<filter_database_handler*>(whdl)->instance);
    return reinterpret_cast<filter_database_sqlite3_handler*>(whdl);
}



void* filter_database_sqlite_initialize(){
    auto res = new filter_database_sqlite3_handler;
    res->op = kautil::database::sqlite3::sqlite_options(); 
    return res;
}


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
        m->create = m->sql->compile(kCreateSt);
        m->insert = m->sql->compile(kInsertSt);
    }
    return 0;
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

int filter_database_sqlite_save(void * whdl){
    auto m=get_instance(whdl);
    m->create->step(true);
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
                    auto res_stmt = !m->insert->set_blob(1,begin_i,block_i) + !m->insert->set_blob(2,begin_o,block_o);
                    auto res_step = m->insert->step(true);
                    res_step = res_step != m->op->sqlite_ok();
                    if((fail=res_stmt+res_step))break;
                }
                if(fail) m->sql->roll_back();
                m->sql->end_transaction();
                return !fail;
            }
        }
    }else m->sql->error_msg();
    return 1;
}

//
//int filter_database_sqlite(filter* f){
//    auto db=reinterpret_cast<filter_database_sqlite3_handler*>(f->database);
//    if(!m->sql) {
//        auto path = f->hdl->local_uri +"/" + f->id_hr(f) + ".sqlite";
//        {
//            struct stat st;
//            auto buf = std::string(path.data());
//            auto dir = dirname(buf.data());
//            if(stat(dir,&st)){
//                if(mkdir(dir)){
//                    printf("fail");
//                    return 1;
//                }
//            } 
//        }
//        m->op = kautil::database::sqlite3::sqlite_options(); 
//        m->sql = new kautil::database::Sqlite3{path.data(),m->op->sqlite_open_create()|m->op->sqlite_open_readwrite()|m->op->sqlite_open_nomutex()};
//        m->create = m->sql->compile(kCreateSt);
//        m->insert = m->sql->compile(kInsertSt);
//    }
//
//
//    m->create->step(true);
//    if(auto begin_i = reinterpret_cast<char*>(f->input(f))){
//        auto end_i = reinterpret_cast<char*>(f->input(f)) + f->input_bytes(f);
//        auto block_i = (end_i - begin_i) / f->hdl->io_len;
//
//        if(auto begin_o = reinterpret_cast<const char*>(f->output(f))){
//            auto end_o = reinterpret_cast<const char*>(f->output(f)) + f->output_bytes(f);
//            auto block_o = (end_o - begin_o) / f->hdl->io_len;
//            auto fail = false;
//            m->sql->begin_transaction();
//            for(;begin_i != end_i; begin_i+=block_i,begin_o+=block_o){
//                auto res_stmt = !m->insert->set_blob(1,begin_i,block_i) + !m->insert->set_blob(2,begin_o,block_o);
//                auto res_step = m->insert->step(true);
//                res_step = res_step != m->op->sqlite_ok();
//                if((fail=res_stmt+res_step))break;
//            }
//            if(fail) m->sql->roll_back();
//            m->sql->end_transaction();
//            return 0;
//        }
//        return 1;
//    }else m->sql->error_msg();
//    return 1;
//}




struct filter_lookup_table;
struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(8)));

void * filter_lookup(filter_lookup_table * flookup_table,const char * key){
    // sorting is bad for this process. i neither want  not to change the order of the flookup_table nor to copy it 
    auto arr = reinterpret_cast<char **>(flookup_table);
    for(;;arr+=sizeof(filter_lookup_elem)/sizeof(uintptr_t)){
        if(nullptr==arr[0]) break;
        if(!strcmp(key,arr[0]))return reinterpret_cast<void*>(arr[1]);
    }
    return nullptr;
}



struct subtract{
    std::vector<double> res;
    uint64_t len=0;
};

#define m(object) reinterpret_cast<subtract*>(object->m)

struct filter_lookup_table{
    filter_lookup_elem main{.key="fmain"};
    filter_lookup_elem output{.key="output"};
    filter_lookup_elem output_bytes{.key="output_bytes"};
    filter_lookup_elem input{.key="input"};
    filter_lookup_elem input_bytes{.key="input_bytes"};
    filter_lookup_elem database_type{.key="database_type"};
    filter_lookup_elem id{.key="id"};
    filter_lookup_elem id_hr{.key="id_hr"};
    filter_lookup_elem member{.key="member"};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(8)));

extern "C" int fmain(filter * f) {
    auto arr = reinterpret_cast<double*>(f->input(f));
    auto len = f->input_bytes(f)/sizeof(double);
    
    auto a = m(f)->len;
    m(f)->res.push_back(100);
    
    if(m(f)->res.size() < len) m(f)->res.resize(len);
    m(f)->len=len;
    for(auto i = 0; i < len ; i++){
        m(f)->res[i] = arr[i];
    }
    return 0;
}




extern "C" uint64_t output_bytes(filter * f) { return m(f)->res.size()*sizeof(double); }
extern "C" void* output(filter * f) { return m(f)->res.data(); }
extern "C" const char* id(filter * f) { return FILTER_ID; }
extern "C" const char* id_hr(filter * f) { return FILTER_ID_HR; }
extern "C" int database_type(filter * f){ return FILTER_DATABASE_TYPE_SQLITE3; }


filter_lookup_table * filter_lookup_table_initialize(){ 
    auto res= new filter_lookup_table{}; 
    res->main.value = (void*)fmain;
    res->output.value = (void*)output;
    res->output_bytes.value=(void*)output_bytes;
    res->id.value=(void*)id;
    res->id_hr.value=(void*)id_hr;
    res->member.value = new subtract{};
    res->database_type.value = (void *) database_type;
    return res;
}

void filter_lookup_table_free(filter_lookup_table * f){
    delete reinterpret_cast<subtract*>(f->member.value);
    delete f; 
}


#define __begin_cycle
#define __end_cycle



struct filter_first{
    void * o=0;
    uint64_t o_bytes=0;
    uint64_t o_length=0;
}__attribute__((aligned(8)));

void* filter_first_output(filter * f){
    return reinterpret_cast<filter_first*>(f->m)->o;
}
uint64_t filter_first_output_bytes(filter * f){
    return reinterpret_cast<filter_first*>(f->m)->o_bytes;
}

uint64_t filter_first_output_length(filter * f){
    return reinterpret_cast<filter_first*>(f->m)->o_length;
}

filter_database_handler* filter_database_handler_initialize(filter * f){
    if(f->database_type){
        switch(f->database_type(f)){
            case FILTER_DATABASE_TYPE_SQLITE3:{
                auto res = new filter_database_handler{};
                res->type = f->database_type(f);
                res->instance = filter_database_sqlite_initialize();
                return res;
            };
        }
    }// database_type
    return nullptr;
}


void filter_database_handler_free(filter * f){
    if(f->db) {
        if (f->database_type) {
            switch (f->database_type(f)){
                case FILTER_DATABASE_TYPE_SQLITE3:filter_database_sqlite_free(f->db->instance);
            };
        } // database_type
    }// db
}

int main(){
    
    auto input_len = 100; // all the input/output inside a chain is the same,if the result structure are counted as one data 
    auto i = new filter_first{};
    filter input;{
        {
            auto arr = new double[input_len];
            for(auto i = 0; i < input_len; ++i)arr[i] = i;
            i->o = arr;
            i->o_bytes = sizeof(double)*input_len; 
            i->o_length = input_len; 
        }
        input.m = i;
        input.output=filter_first_output;
        input.output_bytes=filter_first_output_bytes;
    }
    
    
    using output_t = void*(*)(filter *);
    using output_bytes_t = uint64_t(*)(filter *);
    using filter_id_t = const char* (*)(filter *);
    using database_type_t = int (*)(filter *);

    auto f = filter{};
    auto fhdl = filter_handler{};
    fhdl.io_len = input_len;
    fhdl.local_uri = "./";
    
    auto flookup = filter_lookup_table_initialize();
    f.hdl=&fhdl;
    f.main= (decltype(f.main))filter_lookup(flookup,"fmain");
    f.output= (output_t)filter_lookup(flookup,"output");
    f.output_bytes= (output_bytes_t)filter_lookup(flookup,"output_bytes");
    f.id= (filter_id_t)filter_lookup(flookup,"id");
    f.id_hr= (filter_id_t)filter_lookup(flookup,"id_hr");
    f.database_type =(database_type_t) filter_lookup(flookup,"database_type");
    f.m= filter_lookup(flookup,"member");
    f.db = filter_database_handler_initialize(&f);
    
    f.db->set_io_length(f.db,100);
    
    
    fhdl.filters.push_back(&input);
    fhdl.filters.push_back(&f);
    {
        fhdl.previous= nullptr;
        fhdl.current= &input;
    }
    
    
    for(auto i = 1; i < fhdl.filters.size(); ++i){
        auto f = fhdl.filters[i];
        f->hdl->previous = f->hdl->current;
        f->hdl->current = f;
        
        f->main(f);
        if(f->db){
            
            f->db->set_uri(f->db,f->hdl->local_uri.data(),f->id_hr(f));
            f->db->set_io_length(f->db,fhdl.io_len);
            auto out = (const char *) f->output(f);
            f->db->set_output(f->db,out,out+f->output_bytes(f));

            auto in = (const char *) f->input(f);
            f->db->set_input(f->db,in,in+f->input_bytes(f));

            f->db->setup(f->db);
            f->db->save(f->db);
            
        }
    }
    
    if(f.output_bytes){
        printf("%lld\n",f.output_bytes(&f));
    }
    if(f.input_bytes){
        f.input_bytes(&f);
        printf("%lld\n",f.input_bytes(&f));
    }

    filter_database_handler_free(&f);
    filter_lookup_table_free(flookup);
    delete [] (double*)i->o;
    delete i;
    
    
    return 0;
}

#endif