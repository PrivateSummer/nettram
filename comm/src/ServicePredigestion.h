#pragma once

#define NT_SERVICE_FACTORY(X) X##ServiceFactory

#define NT_DECLARE_SERVICE(X) \
class NT_SERVICE_FACTORY(X): public nt::ServiceFactory \
{ \
public: \
    virtual nt::LogicService *Create(); \
    virtual const char *Name(); \
};

#define NT_DEFINE_SERVICE(X, N) \
LogicService *NT_SERVICE_FACTORY(X)::Create() \
{ \
	return new X(); \
} \
const char *NT_SERVICE_FACTORY(X)::Name() \
{ \
	return N; \
}

#define NT_MAKE_SERVICE_FACTORY(X) new NT_SERVICE_FACTORY(X)

#define NT_GLOBAL_HOLDER_FEATURE_BASE(X, WORK) \
class X##_global_holder_feature \
{ \
public: \
    X##_global_holder_feature() \
	{ \
	    WORK \
	} \
} X##_global_holder_feature_obj0;

#define NT_GLOBAL_HOLDER_FEATURE_SERVICE(X) \
NT_GLOBAL_HOLDER_FEATURE_BASE(X##ServiceFactory, StandaloneHttpSvr::Instance()->AddServiceFactory(NT_MAKE_SERVICE_FACTORY(X));)


#define NT_RequestMapping(CLASS, URI) \
NT_DECLARE_SERVICE(CLASS) \
NT_DEFINE_SERVICE(CLASS, URI) \
NT_GLOBAL_HOLDER_FEATURE_SERVICE(CLASS)


#define NT_GLOBAL_HOLDER_FEATURE_INTERCEPTOR(X) \
NT_GLOBAL_HOLDER_FEATURE_BASE(X##HandlerInterceptor, StandaloneHttpSvr::Instance()->AddInterceptor(#X, new X());)

#define NT_InterceptorMapping(X) \
NT_GLOBAL_HOLDER_FEATURE_INTERCEPTOR(X)
