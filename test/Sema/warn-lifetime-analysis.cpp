// RUN: %clang_cc1 -fsyntax-only -verify -Wlifetime %s
struct S {
  ~S();
  int m;
  int f();
};

void deref_uninitialized() {
  int *p; // expected-note {{it was never initialized here}}
  *p = 3; // expected-warning {{dereferencing a dangling pointer}}
}

void deref_nullptr() {
  int *q = nullptr;
  *q = 3; // expected-warning {{dereferencing a null pointer}}
}

void ref_leaves_scope() {
  int *p;
  {
    int i = 0;
    p = &i;
    *p = 2; // OK
  }         // expected-note {{pointee 'i' left the scope here}}
  *p = 1;   // expected-warning {{dereferencing a dangling pointer}}
}

void ref_to_member_leaves_scope_call() {
  S *p;
  {
    S s;
    p = &s;
    p->f();     // OK
  }             // expected-note 3 {{pointee 's' left the scope here}}
  p->f();       // expected-warning {{dereferencing a dangling pointer}}
  int i = p->m; // expected-warning {{dereferencing a dangling pointer}}
  p->m = 4;     // expected-warning {{dereferencing a dangling pointer}}
}

// No Pointer involved, thus not checked
void ignore_access_on_non_ref_ptr() {
  S s;
  s.m = 3;
  s.f();
}

// Note: the messages below are for the template instantiation in instantiate_ref_leaves_scope_template
// The checker only checks instantiations
template <typename T>
void ref_leaves_scope_template() {
  T p;
  {
    int i = 0;
    p = &i;
    *p = 2; // OK
  }         // expected-note {{pointee 'i' left the scope here}}
  *p = 1;   // expected-warning {{dereferencing a dangling pointer}}
}

void instantiate_ref_leaves_scope_template() {
  ref_leaves_scope_template<int *>(); // expected-note {{in instantiation of}}
}

int global_i = 4;
int *global_init_p = &global_i; // OK
int *global_uninit_p;           // TODO expected-warning {{the pset of 'global_uninit_p' must be a subset of {(static), (null)}, but is {(invalid)}}
int *global_null_p = nullptr;   // OK

void uninitialized_static() {
  static int *p; // expected-warning {{the pset of 'p' must be a subset of {(static), (null)}, but is {(invalid)}}
}
