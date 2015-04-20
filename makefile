##############################################################
# Change BSTINCL (boost include) and BSTLIB (boost libs) according to your setup.
# Dan Krupsky
# SPBU, 2015
##############################################################

BSTINCL = /home/dankrupsky/boost_1_57_0/install-dir/include
BSTLIB = /home/dankrupsky/boost_1_57_0/install-dir/lib

##############################################################

all:
	g++ main.cpp pbfile.pb.cc md6/md6_compress.c -lboost_filesystem -lboost_system -lprotobuf -I$(BSTINCL) -L$(BSTLIB) -std=c++11 -o dankrupsky
