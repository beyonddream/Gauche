/*
 * class.h - Gauche object system private header
 *
 *   Copyright (c) 2000-2003 Shiro Kawai, All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  $Id: class.h,v 1.33 2003-10-23 14:06:02 shirok Exp $
 */

#ifndef GAUCHE_CLASS_H
#define GAUCHE_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SlotAccessor
 *  - Packages slot initialization and accessing methods.
 */
typedef struct ScmSlotAccessorRec {
    SCM_HEADER;
    ScmClass *klass;            /* the class this accessor belongs to.
                                   need to be checked before used, for the
                                   class may be changed. */
    ScmObj name;                /* slot name (symbol) */
    ScmObj (*getter)(ScmObj instance); /* getter for C accessor */
    void (*setter)(ScmObj instance, ScmObj value); /* setter for C accessor */
    ScmObj initValue;           /* :init-value */
    ScmObj initKeyword;         /* :init-keyword */
    ScmObj initThunk;           /* :initform or :init-thunk */
    int initializable;          /* is this slot initializable? */
    int slotNumber;             /* for :instance slot access */
    ScmObj schemeAccessor;      /* for :virtual slot (getter . setter) */
} ScmSlotAccessor;

typedef ScmObj (*ScmNativeGetterProc)(ScmObj);
typedef void   (*ScmNativeSetterProc)(ScmObj, ScmObj);

SCM_CLASS_DECL(Scm_SlotAccessorClass);
#define SCM_CLASS_SLOT_ACCESSOR    (&Scm_SlotAccessorClass)
#define SCM_SLOT_ACCESSOR(obj)     ((ScmSlotAccessor*)obj)
#define SCM_SLOT_ACCESSOR_P(obj)   SCM_XTYPEP(obj, SCM_CLASS_SLOT_ACCESSOR)

/* for static declaration of fields */
struct ScmClassStaticSlotSpecRec {
    const char *name;
    ScmSlotAccessor accessor;
};

#define SCM_CLASS_SLOT_SPEC(name, getter, setter)		\
    { name, { {SCM_CLASS_STATIC_PTR(Scm_SlotAccessorClass)},    \
              NULL, NULL,					\
              (ScmNativeGetterProc)getter,			\
              (ScmNativeSetterProc)setter,			\
              SCM_UNBOUND,					\
              SCM_FALSE,					\
              SCM_FALSE,					\
              TRUE, -1,						\
              SCM_FALSE,					\
             } }

#define SCM_CLASS_SLOT_SPEC_END()   { NULL }

/* cliche in allocate method */
#define SCM_ALLOCATE(klassname, klass) \
    ((klassname*)Scm_AllocateInstance(klass, sizeof(klassname)))

/* some internal methods */
    
SCM_EXTERN ScmObj Scm_AllocateInstance(ScmClass *klass, int coresize);
SCM_EXTERN ScmObj Scm_ComputeCPL(ScmClass *klass);
SCM_EXTERN ScmObj Scm_ComputeApplicableMethods(ScmGeneric *gf,
					       ScmObj *args,
					       int nargs);
SCM_EXTERN ScmObj Scm_SortMethods(ScmObj methods, ScmObj *args, int nargs);
SCM_EXTERN ScmObj Scm_MakeNextMethod(ScmGeneric *gf, ScmObj methods,
				     ScmObj *args, int nargs, int copyArgs);
SCM_EXTERN ScmObj Scm_AddMethod(ScmGeneric *gf, ScmMethod *method);

SCM_EXTERN ScmObj Scm_VMSlotRefUsingAccessor(ScmObj obj,
					     ScmSlotAccessor *acc,
					     int boundp);
SCM_EXTERN ScmObj Scm_VMSlotSetUsingAccessor(ScmObj obj,
					     ScmSlotAccessor *acc,
					     ScmObj val);

SCM_EXTERN ScmObj Scm_InstanceSlotRef(ScmObj obj, int number);
SCM_EXTERN void Scm_InstanceSlotSet(ScmObj obj, int number, ScmObj val);

SCM_EXTERN int  Scm_StartClassRedefinition(ScmClass *klass);
SCM_EXTERN void Scm_CommitClassRedefinition(ScmClass *klass, ScmClass *newk);
SCM_EXTERN void Scm_AddDirectSubclass(ScmClass *super, ScmClass *sub);
SCM_EXTERN void Scm_RemoveDirectSubclass(ScmClass *super, ScmClass *sub);
SCM_EXTERN void Scm_AddDirectMethod(ScmClass *super, ScmMethod *m);
SCM_EXTERN void Scm_RemoveDirectMethod(ScmClass *super, ScmMethod *m);

SCM_EXTERN ScmObj Scm__InternalClassName(ScmClass *klass);

SCM_EXTERN ScmGeneric Scm_GenericApplyGeneric;
SCM_EXTERN ScmGeneric Scm_GenericObjectHash;
SCM_EXTERN ScmGeneric Scm_GenericObjectApply;
SCM_EXTERN ScmGeneric Scm_GenericObjectSetter;

#ifdef __cplusplus
}
#endif

#endif /* GAUCHE_CLASS_H */
