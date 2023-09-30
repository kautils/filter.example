#include <vector>
#include <string>
#include "flow.h"

struct subtract{
    std::vector<double> res;
    uint64_t len=0;
};

struct filter_lookup_elem{
    const char * key = 0; 
    void * value = nullptr;
}__attribute__((aligned(sizeof(uintptr_t))));

#define m(object) reinterpret_cast<subtract*>(object->m)

struct filter_lookup_table_subtract{
    filter_lookup_elem main{.key="fmain"};
    filter_lookup_elem output{.key="output"};
    filter_lookup_elem output_bytes{.key="output_bytes"};
    filter_lookup_elem database_type{.key="database_type"};
    filter_lookup_elem id{.key="id"};
    filter_lookup_elem id_hr{.key="id_hr"};
    filter_lookup_elem member{.key="member"};
    filter_lookup_elem sentinel{.key=nullptr,.value=nullptr};
} __attribute__((aligned(sizeof(uintptr_t))));
    
int fmain(filter * f) {
    auto arr = reinterpret_cast<double*>(f->input(f));
    auto len = f->input_bytes(f)/sizeof(double);
    
    auto a = m(f)->len;
    m(f)->res.push_back(100);
    
    if(m(f)->res.size() < len) m(f)->res.resize(len);
    m(f)->len=len;
    for(auto i = 0; i < len ; i++){
        m(f)->res[i] = arr[i]+123456789;
    }
    return 0;
}


uint64_t output_bytes(filter * f) { return m(f)->res.size()*sizeof(double); }
void* output(filter * f) { return m(f)->res.data(); }
const char* id(filter * f) { return FILTER_ID; }
const char* id_hr(filter * f) { return FILTER_ID_HR; }
int database_type(filter * f){ return FILTER_DATABASE_TYPE_SQLITE3; }



extern "C" uint64_t size_of_pointer(){ return sizeof(uint64_t); }
extern "C" filter_lookup_table * lookup_table_initialize(){ 
    auto res= new filter_lookup_table_subtract{}; 
    res->main.value = (void*)fmain;
    res->output.value = (void*)output;
    res->output_bytes.value=(void*)output_bytes;
    res->id.value=(void*)id;
    res->id_hr.value=(void*)id_hr;
    res->member.value = new subtract{};
    res->database_type.value = (void *) database_type;
    return reinterpret_cast<filter_lookup_table*>(res);
}

extern "C" void lookup_table_free(filter_lookup_table * f){
    auto entity = reinterpret_cast<filter_lookup_table_subtract*>(f);
    delete reinterpret_cast<subtract*>(entity->member.value);
    delete entity; 
}
