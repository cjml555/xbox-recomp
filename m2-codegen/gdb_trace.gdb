set pagination off
set confirm off
break rex::system::UserModule::LoadFromMemory
run codegen --ignore-stamp
print "LoadFromMemory called"
x/4wx addr
x/16bx addr
p (char*)addr
p length
info share
bt
quit
