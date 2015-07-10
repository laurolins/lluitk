#! /usr/bin/env python
# encoding: utf-8

def options(ctx):
    # print(ctx.__class__.__name__)
    ctx.load('compiler_cxx')
    ctx.load('compiler_c')
    ctx.add_option('--debug', action='store_true', default=False, help='use debug directives')

#    ctx.add_option('--tcmalloc', action='store_true', default=False,
#                help='execute the program after it is built')

def configure(ctx):
    # print(ctx.__class__.__name__)
    ctx.load('compiler_cxx')
    ctx.load('compiler_c')
    ctx.check(features='cxx cxxprogram',msg='cxx and cxxprogram')
    # print('...debug mode? %r' % ctx.options.debug)
    ctx.recurse('examples')        # programs

# def pre(ctx):
#     if ctx.options.debug:
#         OS = open("/tmp/platform","r").readlines()[0].strip()
        
def build(ctx):
    # print(ctx.__class__.__name__)
    # print('...debug mode? %r' % ctx.options.debug)
    ctx.recurse('examples')        # programs





#     bld.recurse('tessellate') # libtessellate

    # conf.env.CFLAGS = ['-g']
    # conf.setenv('release')
    # conf.load('compiler_c')
    # conf.env.CFLAGS = ['-O2']

#, lib=['m'], cflags=['-Wall'], defines=['var=foo'], uselib_store='M')

# def build(bld):
#     # bld(features='c cshlib', source='b.c', target='mylib')
#     # bld(features='cxx cxxprogram', source='rastahit.cc', target='app') #, use=['M','mylib'], lib=['dl'])
#     bld.program(source="rastahit.cc", target='app')

