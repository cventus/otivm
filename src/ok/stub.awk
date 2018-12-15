# parses `nm -PA ...`

function on_symbol(name) { return $2 ~ name && $3 == "T"; }

function stub_unless(name, state) {
  if (!state) {
    printf("void %s(void) { return; }\n", name);
  }
}

on_symbol("^before_tests$") { before = 1; }
on_symbol("^after_tests$") { after = 1; }
on_symbol("^before_each_test$") { before_each = 1; }
on_symbol("^after_each_test$") { after_each = 1; }
on_symbol("^test_") { tests[n++] = $2; }

END {
  printf("#include \"ok/ok.h\"\n\n");

  for (i = 0; i < n; i++) {
    name = tests[i];
    printf("int %s(void);\n", name);
  }

  printf("\nstruct test const tests[] = {\n");
  for (i = 0; i < n; i++) {
    name = tests[i];
    desc = tests[i];
    gsub(/^test_/, "", desc);
    gsub(/_/, " ", desc);
    printf("\t{ %s, \"%s\" },\n", name, desc);
  }
  printf("\t{ 0, 0 }\n};\n\n");
  stub_unless("before_tests", before);
  stub_unless("after_tests", after);
  stub_unless("before_each_test", before_each);
  stub_unless("after_each_test", after_each);
}
