/* File: libfdt.i */
%module libfdt

%{
#define SWIG_FILE_WITH_INIT
#include "libfdt.h"
%}

%pythoncode %{
def Raise(errnum):
    raise ValueError('Error %s' % fdt_strerror(errnum))

def Name(fdt, offset):
    name, len = fdt_get_name(fdt, offset)
    return name

def String(fdt, offset):
    offset = fdt32_to_cpu(offset)
    name = fdt_string(fdt, offset)
    return name

def swap32(x):
    return (((x << 24) & 0xFF000000) |
            ((x <<  8) & 0x00FF0000) |
            ((x >>  8) & 0x0000FF00) |
            ((x >> 24) & 0x000000FF))

def fdt32_to_cpu(x):
    return swap32(x)

def Data(prop):
    set_prop(prop)
    return get_prop_data()
%}

%include "typemaps.i"
%include "cstring.i"

%typemap(in) void* = char*;

typedef int fdt32_t;

struct fdt_property {
        fdt32_t tag;
        fdt32_t len;
        fdt32_t nameoff;
        char data[0];
};

/*
 * This is a work-around since I'm not sure of a better way to copy out the
 * contents of a string. This is used in dtoc/GetProps(). The intent is to
 * pass in a pointer to a property and access the data field at the end of
 * it. Ideally the Data() function above would be able to do this directly,
 * but I'm not sure how to do that.
 */
#pragma SWIG nowarn=454
%inline %{
    static struct fdt_property *cur_prop;

    void set_prop(struct fdt_property *prop) {
        cur_prop = prop;
    }
%}

%cstring_output_allocate_size(char **s, int *sz, free(*$1));
%inline %{
    void get_prop_data(char **s, int *sz) {
        *sz = fdt32_to_cpu(cur_prop->len);
        *s = (char *)malloc(*sz);
        if (!*s)
            *sz = 0;
        else
            memcpy(*s, cur_prop + 1, *sz);
    }
%}

%typemap(in) (const void *) {
  if (!PyByteArray_Check($input)) {
    SWIG_exception_fail(SWIG_TypeError, "in method '" "$symname" "', argument "
                       "$argnum"" of type '" "$type""'");
  }
  $1 = (void *) PyByteArray_AsString($input);
}

const void *fdt_offset_ptr(const void *fdt, int offset, unsigned int checklen);
int fdt_path_offset(const void *fdt, const char *path);
int fdt_first_property_offset(const void *fdt, int nodeoffset);
int fdt_next_property_offset(const void *fdt, int offset);
const char *fdt_strerror(int errval);
const struct fdt_property *fdt_get_property_by_offset(const void *fdt,
                                                      int offset,
                                                      int *OUTPUT);
const char *fdt_get_name(const void *fdt, int nodeoffset, int *OUTPUT);
const char *fdt_string(const void *fdt, int stroffset);
int fdt_first_subnode(const void *fdt, int offset);
int fdt_next_subnode(const void *fdt, int offset);

%typemap(in) (void *) {
  if (!PyByteArray_Check($input)) {
    SWIG_exception_fail(SWIG_TypeError, "in method '" "$symname" "', argument "
                       "$argnum"" of type '" "$type""'");
  }
  $1 = PyByteArray_AsString($input);
}

int fdt_delprop(void *fdt, int nodeoffset, const char *name);

const char *fdt_strerror(int errval);
int fdt_pack(void *fdt);

int fdt_totalsize(const void *fdt);
int fdt_off_dt_struct(const void *fdt);
