#ifndef BEAN_RUNTIME_H
#define BEAN_RUNTIME_H

#include "bean/types.h"

#include <Block.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__arm64__) || defined(__aarch64__)
#define bean_msgSend objc_msgSend
#define bean_msgSend_stret objc_msgSend
#define bean_msgSend_fpret objc_msgSend
#elif defined(__x86_64__)
#define bean_msgSend objc_msgSend
#define bean_msgSend_stret objc_msgSend_stret
#define bean_msgSend_fpret objc_msgSend_fpret
#else
#define bean_msgSend objc_msgSend
#define bean_msgSend_stret objc_msgSend_stret
#define bean_msgSend_fpret objc_msgSend_fpret
#endif

static inline bean_sel bean_sel_register(const char* name) {
    return sel_registerName(name);
}

#define bean_sel(name) sel_registerName(name)

static inline bean_class bean_get_class(const char* name) {
    return (bean_class)objc_getClass(name);
}

#define bean_cls(name) ((bean_class)objc_getClass(name))

static inline bean_id bean_alloc(bean_class cls) {
    return ((bean_id (*)(bean_class, bean_sel))objc_msgSend)(cls, bean_sel("alloc"));
}

static inline bean_id bean_init(bean_id obj) {
    return ((bean_id (*)(bean_id, bean_sel))objc_msgSend)(obj, bean_sel("init"));
}

static inline bean_id bean_retain(bean_id obj) {
    return ((bean_id (*)(bean_id, bean_sel))objc_msgSend)(obj, bean_sel("retain"));
}

static inline void bean_release(bean_id obj) {
    ((void (*)(bean_id, bean_sel))objc_msgSend)(obj, bean_sel("release"));
}

static inline bean_id bean_autorelease(bean_id obj) {
    return ((bean_id (*)(bean_id, bean_sel))objc_msgSend)(obj, bean_sel("autorelease"));
}

typedef struct bean_autorelease_pool_scope {
    bean_id pool;
} bean_autorelease_pool_scope;

static inline bean_autorelease_pool_scope bean_autorelease_pool_push(void) {
    bean_class pool_class = bean_get_class("NSAutoreleasePool");
    bean_id pool = bean_alloc(pool_class);
    pool = ((bean_id (*)(bean_id, bean_sel))objc_msgSend)(pool, bean_sel("init"));
    bean_autorelease_pool_scope scope = {pool};
    return scope;
}

static inline void bean_autorelease_pool_pop(bean_autorelease_pool_scope scope) {
    if (scope.pool) {
        bean_release(scope.pool);
    }
}

static inline bean_id bean_nsstring_from_utf8(const char* utf8) {
    if (!utf8) {
        return NULL;
    }
    bean_class cls = bean_get_class("NSString");
    return ((bean_id (*)(bean_class, bean_sel, const char*))objc_msgSend)(
        cls, bean_sel("stringWithUTF8String:"), utf8);
}

static inline const char* bean_nsstring_to_utf8(bean_id str) {
    if (!str) {
        return NULL;
    }
    return ((const char* (*)(bean_id, bean_sel))objc_msgSend)(str, bean_sel("UTF8String"));
}

typedef struct bean_method_spec {
    const char* selector;
    IMP implementation;
    const char* type_encoding;
} bean_method_spec;

static inline bean_class bean_make_subclass(
    const char* name,
    bean_class superclass,
    const bean_method_spec* methods,
    size_t method_count,
    size_t ivar_size
) {
    bean_class cls = objc_allocateClassPair(superclass, name, (unsigned)ivar_size);
    if (!cls) {
        return NULL;
    }

    for (size_t i = 0; i < method_count; ++i) {
        const bean_method_spec* spec = &methods[i];
        if (!class_addMethod(cls, sel_registerName(spec->selector), spec->implementation, spec->type_encoding)) {
            objc_disposeClassPair(cls);
            return NULL;
        }
    }

    objc_registerClassPair(cls);
    return cls;
}

static inline void* bean_object_get_ivar(bean_id obj, const char* name) {
    void* value = NULL;
    object_getInstanceVariable(obj, name, &value);
    return value;
}

static inline BOOL bean_object_set_ivar(bean_id obj, const char* name, void* value) {
    return object_setInstanceVariable(obj, name, value) != NULL;
}

typedef void (*bean_block_fn0)(void* context);
typedef void (*bean_block_fn1)(void* context, bean_id arg0);
typedef void (*bean_block_fn2)(void* context, bean_id arg0, bean_id arg1);

struct bean_block_literal_0 {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_0*, void* context);
    struct {
        unsigned long int reserved;
        unsigned long int size;
        void (*copy_helper)(void* dst, const void* src);
        void (*dispose_helper)(const void* src);
        const char* signature;
    } descriptor;
    void* context;
};

struct bean_block_literal_1 {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_1*, void* context, bean_id arg0);
    struct {
        unsigned long int reserved;
        unsigned long int size;
        void (*copy_helper)(void* dst, const void* src);
        void (*dispose_helper)(const void* src);
        const char* signature;
    } descriptor;
    void* context;
};

static inline void* bean_block_stack_isa(void) {
    return (void*)objc_getClass("_NSConcreteStackBlock");
}

static inline void* bean_block_create_0(void* context, void (*fn)(void*, void*)) {
    struct bean_block_literal_0 block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = (1 << 25) | (1 << 26);
    block.invoke = (void (*)(struct bean_block_literal_0*, void*))fn;
    block.context = context;
    block.descriptor.size = sizeof(struct bean_block_literal_0);
    return Block_copy(&block);
}

static inline void* bean_block_create_1(void* context, void (*fn)(void*, void*, bean_id)) {
    struct bean_block_literal_1 block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = (1 << 25) | (1 << 26);
    block.invoke = (void (*)(struct bean_block_literal_1*, void*, bean_id))fn;
    block.context = context;
    block.descriptor.size = sizeof(struct bean_block_literal_1);
    return Block_copy(&block);
}

#ifdef __cplusplus
}
#endif

#endif /* BEAN_RUNTIME_H */
