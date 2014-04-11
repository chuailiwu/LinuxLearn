include ../Make.defines.linux

SRCS=$(wildcard *.cpp)
#OBJS = $(SRCS:%.cpp=%.o)
#OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPE=$(SRCS:%.cpp=$(DEPDIR)/%.d)
#DEPE=.dep/public.d

#BASE = $(basename $(SRCS))
#OBJS = $(addsuffix .o, $(addprefix .obj/,$(BASE)))
#DEPS = $(addsuffix .d, $(addprefix dep/,$(BASE)))

ALL_TARGETS = public daemonize upperstr test18 ruptime ruptimed unixbind getpass prwinsize
# Our library that almost every program needs.

# Common temp files to delete from each directory.
TEMPFILES=core core.* *.o temp.* *.out typescript*


AR := ar
RANLIB := ranlib
RM := rm -f


#vpath %.h ../headers 指定的路径仅限于在Makefile文件内容中出现的.h。 不能指定源文件中包含的头文件所在路径(.c文件中所包含的头文件路径需要使用gcc的“-I”选项来指定)


#通过“.PHONY”特殊目标将“clean”目标声明为目标。避免当磁盘上存在一个名为“clean”文件时，目标“clean”所在规则的命令无法执行
.PHONY : all clean
all: $(ALL_TARGETS)

#.obj/%.o: %.cpp
#    $(CXX) -c -o .obj/$@ $(INCDIR) $(CXXFLAGS) $<
#sed... $(OBJDIR)是obj目录，根据实际替换($(OBJDIR)/\1)，$@是makefile里的目标
#如果上一条命令的结果应用在下一条命令时，即放在一个subshell当中执行，应该使用分号分隔这两条命令，以下%.d: %.c依赖关系用了分号
#%.cpp前不能加路径
$(DEPE):$(DEPDIR)/%.d:%.cpp
	@set -e;rm -f $@;\
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o : ,g;a\\t$$(CXX) -c $$(CPPFLAGS) -o $$(OBJDIR)/$*.o $<' < $@.$$$$ > $@;\
	rm -f $@.$$$$

#没有这句无法生成.d文件，注意$(DEPE)路径必须与上面的一致
ifneq ($(MAKECMDGOALS),clean) 
sinclude $(DEPE)
endif 

#sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g;a\\t$$(CXX) -c $$(CPPFLAGS) -o $$(OBJDIR)/$*.o $<' < $@.$$$$ > $@;\
#生成如下是不行的，因为.dep/public.d变成有两个规则来生成了，上面一个，下面一个
#.obj/public.o .dep/public.d : public.cpp ../include/public.h
#       $(CXX) -c $(CPPFLAGS) -o $(OBJDIR)/public.o public.cpp


public: $(OBJDIR)/public.o $(OBJDIR)/single.o $(OBJDIR)/daemonize.o
	${AR} rv $(LIBDIR)/libpublic.a $?
	${RANLIB} $(LIBDIR)/libpublic.a

upperstr: $(OBJDIR)/upperstr.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -lpublic -o $(BINDIR)/$@ 

test18: $(OBJDIR)/test18.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -lpublic -o $(BINDIR)/$@

ruptime: $(OBJDIR)/ruptime.o $(OBJDIR)/cliconn.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -lpublic -o $(BINDIR)/$@

ruptimed: $(OBJDIR)/initsrv.o $(OBJDIR)/ruptimed.o
	$(CXX) $(CXXFLAGS) $(INCDIR) $? $(LDDIR) -lpublic -o $(BINDIR)/$@

unixbind: $(OBJDIR)/unixbind.o
	$(CXX) $(CXXFLAGS) $(INCDIR) $? $(LDDIR) -lpublic -o $(BINDIR)/$@

getpass: $(OBJDIR)/getpass.o
	$(CXX) $(CXXFLAGS) $(INCDIR) $? $(LDDIR) -lpublic -o $(BINDIR)/$@

prwinsize: $(OBJDIR)/prwinsize.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -lpublic -ldaemon -o $(BINDIR)/$@

checkInput: $(OBJDIR)/checkInput.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -o $(BINDIR)/$@

addSub: $(OBJDIR)/addSub.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -o $(BINDIR)/$@

classMemberFunc: $(OBJDIR)/classMemberFunc.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -o $(BINDIR)/$@

singleInstance: $(OBJDIR)/singleInstance.o
	$(CXX) $(CXXFLAGS) $? $(LDDIR) -lpublic -o $(BINDIR)/$@

classCast: $(OBJDIR)/classCast.o
	$(CXX) $(CXXFLAGS) $? -o $(BINDIR)/$@

output: $(OBJDIR)/output.o
	$(CXX) $(CXXFLAGS) $? -o $(BINDIR)/$@

mystl: $(OBJDIR)/mystl.o
	$(CXX) $(CXXFLAGS) $? -o $(BINDIR)/$@

misc: $(OBJDIR)/misc.o
	$(CXX) $(CXXFLAGS) $? -o $(BINDIR)/$@

checkMem: $(OBJDIR)/checkMem.o
	$(CXX) $(CXXFLAGS) $? -o $(BINDIR)/$@


clean:
	$(RM) -f $(OBJDIR)/*.o core $(LIBDIR)/*.a  $(BINDIR)/* $(DEPE)

