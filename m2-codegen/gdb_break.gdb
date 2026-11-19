set pagination off
set confirm off
break rex::system::UserModule::LoadFromMemory
run codegen --ignore-stamp
printf "\n=== HIT LoadFromMemory ===\n"
x/4wx addr
x/16bx addr
printf "len = %zu\n", (size_t)length
printf "this = %p\n", (void*)this
continue
printf "\n=== DONE ===\n"
quit
