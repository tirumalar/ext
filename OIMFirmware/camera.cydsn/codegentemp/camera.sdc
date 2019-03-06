# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\psoc\chamonix\camera.cydsn\camera.cyprj
# Date: Mon, 12 Nov 2018 14:30:41 GMT
#set_units -time ns
create_clock -name {Clock_1(FFB)} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/ff_div_11}]]
create_clock -name {Clock_2(FFB)} -period 83.333333333333329 -waveform {0 41.6666666666667} [list [get_pins {ClockBlock/ff_div_12}]]
create_clock -name {ADC_intClock(FFB)} -period 1000 -waveform {0 500} [list [get_pins {ClockBlock/ff_div_10}]]
create_clock -name {EZI2C_SCBCLK(FFB)} -period 125 -waveform {0 62.5} [list [get_pins {ClockBlock/ff_div_3}]]
create_clock -name {CyRouted1} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/dsi_in_0}]]
create_clock -name {CyILO} -period 31250 -waveform {0 15625} [list [get_pins {ClockBlock/ilo}]]
create_clock -name {CyLFCLK} -period 31250 -waveform {0 15625} [list [get_pins {ClockBlock/lfclk}]]
create_clock -name {CyIMO} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyHFCLK} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/hfclk}]]
create_clock -name {CySYSCLK} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/sysclk}]]
create_generated_clock -name {Clock_1} -source [get_pins {ClockBlock/hfclk}] -edges {1 2 3} [list]
create_generated_clock -name {Clock_2} -source [get_pins {ClockBlock/hfclk}] -edges {1 5 9} [list]
create_generated_clock -name {ADC_intClock} -source [get_pins {ClockBlock/hfclk}] -edges {1 49 97} [list]
create_generated_clock -name {EZI2C_SCBCLK} -source [get_pins {ClockBlock/hfclk}] -edges {1 7 13} [list]

set_false_path -from [get_pins {__ONE__/q}]

# Component constraints for C:\psoc\chamonix\camera.cydsn\TopDesign\TopDesign.cysch
# Project: C:\psoc\chamonix\camera.cydsn\camera.cyprj
# Date: Mon, 12 Nov 2018 14:30:38 GMT
