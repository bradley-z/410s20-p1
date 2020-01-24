/** @file spec/compiler.h
 *  @brief Useful macros.  Inspired by Linux compiler*.h and kernel.h.
 *  @author bblum S2011
 */

#ifndef _SPEC_COMPILER_H
#define _SPEC_COMPILER_H

/* Function annotations */
#ifndef NORETURN // noreturn might also be defined in spec/syscall.h...
# define NORETURN __attribute__((noreturn))
#endif

# define MUST_CHECK __attribute__((warn_unused_result))

/* Force a compilation error if condition is false */
#define STATIC_ZERO_ASSERT(condition) switch(0){case 0:case (condition):;}
#define STATIC_NULL_ASSERT(condition) STATIC_ZERO_ASSERT(condition)

/* Force a compilation error if condition is false */
#define STATIC_ASSERT(condition) STATIC_ZERO_ASSERT(condition)

#endif /* !_SPEC_COMPILER_H */
