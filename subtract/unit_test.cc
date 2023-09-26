
#ifdef TMAIN_KAUTIL_FILTER_ALTHMETIC_SUBTRACT_SHARED

#include <vector>
#include <string>
#include "kautil/cache/virtual_file/virtual_file.h"
#include "flow.h"

// +++++++++++++++++++++ temporal +++++++++++++++++++++ 
// +++++++++++++++++++++ temporal +++++++++++++++++++++ 
// +++++++++++++++++++++ temporal +++++++++++++++++++++ 
struct filter_handler{ 
    filter * previous=0;
    filter * current=0;
    std::vector<filter*> filters;
    std::string local_uri;
    uint64_t io_len=0;
};
// +++++++++++++++++++++ temporal +++++++++++++++++++++ 
// +++++++++++++++++++++ temporal +++++++++++++++++++++ 
// +++++++++++++++++++++ temporal +++++++++++++++++++++ 



struct subtract{
    std::vector<double> res;
    uint64_t len=0;
};

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
} __attribute__((aligned(8)));

extern "C" int fmain(filter * f) {
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




extern "C" uint64_t output_bytes(filter * f) { 
    return m(f)->res.size()*sizeof(double); 
}
extern "C" void* output(filter * f) { 
    return m(f)->res.data(); 
}
extern "C" const char* id(filter * f) { return FILTER_ID; }
extern "C" const char* id_hr(filter * f) { return FILTER_ID_HR; }
extern "C" int database_type(filter * f){ return FILTER_DATABASE_TYPE_SQLITE3; }


filter_lookup_table * filter_lookup_table_initialize(){ 
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

void filter_lookup_table_free(filter_lookup_table * f){
    auto m = reinterpret_cast<filter_lookup_table_subtract*>(f)->member;
    delete reinterpret_cast<subtract*>(m.value);
    delete f; 
}



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

    auto fhdl = filter_handler_initialize();
    filter_handler_set_io_length(fhdl,input_len);
    filter_handler_set_local_uri(fhdl,"./");
    auto flookup = filter_lookup_table_initialize();
    
    auto f = filter{};{
        f.hdl=fhdl;
        f.main= (decltype(f.main))filter_lookup(flookup,"fmain");
        f.output= (output_t)filter_lookup(flookup,"output");
        f.output_bytes= (output_bytes_t)filter_lookup(flookup,"output_bytes");
        f.id= (filter_id_t)filter_lookup(flookup,"id");
        f.id_hr= (filter_id_t)filter_lookup(flookup,"id_hr");
        f.database_type =(database_type_t) filter_lookup(flookup,"database_type");
        f.m= filter_lookup(flookup,"member");
        f.option=FILTER_DATABASE_OPTION_OVERWRITE | FILTER_DATABASE_OPTION_WITHOUT_ROWID;
    }

    {
        filter_handler_push(fhdl,&input);
        filter_handler_push(fhdl,&f);
    }
    
    f.setup_database(&f);
    for(auto i = 1; i < fhdl->filters.size(); ++i){
        auto f = fhdl->filters[i];
        f->hdl->previous = f->hdl->current;
        f->hdl->current = f;
        f->main(f);
        f->save(f);
    }
    f.db->free(f.db);
    
    filter_lookup_table_free(flookup);
    delete [] (double*)i->o;
    delete i;
    
    filter_handler_free(fhdl);
    
    
    return 0;
}

#endif