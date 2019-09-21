#include "util.h"
using namespace xiao::helper;

void
ConfigStorShow(void)
{
    ns3::GtkConfigStore config;
    config.ConfigureDefaults ();
    config.ConfigureAttributes ();
}


int 
main (int argc, char *argv[])
{

}
