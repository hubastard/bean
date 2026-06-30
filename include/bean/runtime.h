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

static inline bean_id bean_new(bean_class cls) {
    /* alloc + -init. Do not use when a designated initializer follows (e.g. initWithFrame:). */
    return bean_init(bean_alloc(cls));
}

#define autoreleasepool \
    for (bean_autorelease_pool_scope bean_pool__ = bean_autorelease_pool_push(), \
         *bean_pool_ptr__ = &bean_pool__; \
         bean_pool_ptr__; \
         bean_autorelease_pool_pop(*bean_pool_ptr__), bean_pool_ptr__ = NULL)

#define bean_super_struct(self, super_cls) \
    ((struct objc_super){ (self), (Class)(super_cls) })

/* Typed super calls — variadic casts break struct args (CGRect, CGSize) on arm64. */
#define bean_super_call_obj(self, super_cls, cmd, obj) \
    ((void (*)(struct objc_super*, SEL, bean_id))objc_msgSendSuper)( \
        &bean_super_struct((self), (super_cls)), (cmd), (obj))

#define bean_super_call_rect(self, super_cls, cmd, rect) \
    ((void (*)(struct objc_super*, SEL, CGRect))objc_msgSendSuper)( \
        &bean_super_struct((self), (super_cls)), (cmd), (rect))

#define bean_super_call_size(self, super_cls, cmd, size) \
    ((void (*)(struct objc_super*, SEL, CGSize))objc_msgSendSuper)( \
        &bean_super_struct((self), (super_cls)), (cmd), (size))

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

#define bean_str(cstr) bean_nsstring_from_utf8((cstr))

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

typedef void (*bean_block_fn0)(void* context, void* block);
typedef void (*bean_block_fn1)(void* context, void* block, bean_id arg0);
typedef void (*bean_block_fn2)(void* context, void* block, long arg0, bean_id arg1);
typedef void (*bean_block_fn2_id)(void* context, void* block, bean_id arg0, bean_id arg1);
typedef void (*bean_bool_error_fn)(void* context, void* block, BOOL ok, bean_id err);

struct bean_block_literal_bool_error {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_bool_error*, BOOL ok, bean_id err);
    struct bean_block_descriptor* descriptor;
    void* context;
    bean_bool_error_fn user_fn;
};

struct bean_block_descriptor {
    unsigned long int reserved;
    unsigned long int size;
};

struct bean_block_literal_0 {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_0*);
    struct bean_block_descriptor* descriptor;
    void* context;
    bean_block_fn0 user_fn;
};

struct bean_block_literal_1 {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_1*, bean_id arg0);
    struct bean_block_descriptor* descriptor;
    void* context;
    bean_block_fn1 user_fn;
};

struct bean_block_literal_2 {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_2*, long arg0, bean_id arg1);
    struct bean_block_descriptor* descriptor;
    void* context;
    bean_block_fn2 user_fn;
};

struct bean_block_literal_2_id {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(struct bean_block_literal_2_id*, bean_id arg0, bean_id arg1);
    struct bean_block_descriptor* descriptor;
    void* context;
    bean_block_fn2_id user_fn;
};

static void bean_block_invoke_0(struct bean_block_literal_0* block) {
    if (block && block->user_fn) {
        block->user_fn(block->context, block);
    }
}

static void bean_block_invoke_1(struct bean_block_literal_1* block, bean_id arg0) {
    if (block && block->user_fn) {
        block->user_fn(block->context, block, arg0);
    }
}

static void bean_block_invoke_2(struct bean_block_literal_2* block, long arg0, bean_id arg1) {
    if (block && block->user_fn) {
        block->user_fn(block->context, block, arg0, arg1);
    }
}

static void bean_block_invoke_2_id(
    struct bean_block_literal_2_id* block,
    bean_id arg0,
    bean_id arg1
) {
    if (block && block->user_fn) {
        block->user_fn(block->context, block, arg0, arg1);
    }
}

static void bean_block_invoke_bool_error(
    struct bean_block_literal_bool_error* block,
    BOOL ok,
    bean_id err
) {
    if (block && block->user_fn) {
        block->user_fn(block->context, block, ok, err);
    }
}

static inline void* bean_block_stack_isa(void) {
    return _NSConcreteStackBlock;
}

static inline void* bean_block_create_0(void* context, bean_block_fn0 fn) {
    static struct bean_block_descriptor desc = { 0, sizeof(struct bean_block_literal_0) };
    struct bean_block_literal_0 block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = 0;
    block.invoke = bean_block_invoke_0;
    block.descriptor = &desc;
    block.context = context;
    block.user_fn = fn;
    return Block_copy(&block);
}

static inline void* bean_block_create_1(void* context, bean_block_fn1 fn) {
    static struct bean_block_descriptor desc = { 0, sizeof(struct bean_block_literal_1) };
    struct bean_block_literal_1 block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = 0;
    block.invoke = bean_block_invoke_1;
    block.descriptor = &desc;
    block.context = context;
    block.user_fn = fn;
    return Block_copy(&block);
}

static inline void* bean_block_create_2(void* context, bean_block_fn2 fn) {
    static struct bean_block_descriptor desc = { 0, sizeof(struct bean_block_literal_2) };
    struct bean_block_literal_2 block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = 0;
    block.invoke = bean_block_invoke_2;
    block.descriptor = &desc;
    block.context = context;
    block.user_fn = fn;
    return Block_copy(&block);
}

static inline void* bean_block_create_2_id(void* context, bean_block_fn2_id fn) {
    static struct bean_block_descriptor desc = { 0, sizeof(struct bean_block_literal_2_id) };
    struct bean_block_literal_2_id block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = 0;
    block.invoke = bean_block_invoke_2_id;
    block.descriptor = &desc;
    block.context = context;
    block.user_fn = fn;
    return Block_copy(&block);
}

static inline void* bean_block_bool_error(void* context, bean_bool_error_fn fn) {
    static struct bean_block_descriptor desc = { 0, sizeof(struct bean_block_literal_bool_error) };
    struct bean_block_literal_bool_error block = {0};
    block.isa = bean_block_stack_isa();
    block.flags = 0;
    block.invoke = bean_block_invoke_bool_error;
    block.descriptor = &desc;
    block.context = context;
    block.user_fn = fn;
    return Block_copy(&block);
}

#ifdef __cplusplus
}
#endif

#endif /* BEAN_RUNTIME_H */
