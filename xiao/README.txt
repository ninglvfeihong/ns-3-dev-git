using below command to help debug
./waf --run lr-wpan-error-distance-plot 'NS_LOG=*=level_warn'
./waf --run lr-wpan-error-distance-plot 'NS_LOG=LrWpanNetDevice=level_warn'
Other environment may include:
'NS_LOG=*=level_all|prefix_func|prefix_time'
'NS_LOG=LrWpanNetDevice=level_all|prefix_func|prefix_time'


Useful command:
1. ./waf --run scratch/lrwpan_test 'NS_LOG=*=level_warn|prefix_time'

