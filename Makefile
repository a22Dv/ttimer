NAME := ttimer
PREFIX ?= /usr/local
BINDIR  := $(PREFIX)/bin
CXX := g++

CXXFLAGS += -std=c++23 -O3 -s 
LDFLAGS += -static-libgcc -static-libstdc++

SRCDIR := src
BUILDDIR := build
INCDIR := include

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
INCS := $(wildcard $(INCDIR)/*.hpp)

.PHONY: all clean install
	
all: $(NAME)

$(NAME): $(OBJS) $(INCS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
		$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR):
		mkdir -p $(BUILDDIR)
		
install: all
		install -Dm755 $(NAME) $(DESTDIR)$(BINDIR)/$(NAME)

clean:
		rm -rf $(BUILDDIR) $(NAME)


