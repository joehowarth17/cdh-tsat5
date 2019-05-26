set_device \
    -family  SmartFusion2 \
    -die     PA4M1000_N \
    -package tq144 \
    -speed   STD \
    -tempr   {COM} \
    -voltr   {COM}
set_def {VOLTAGE} {1.2}
set_def {VCCI_1.2_VOLTR} {COM}
set_def {VCCI_1.5_VOLTR} {COM}
set_def {VCCI_1.8_VOLTR} {COM}
set_def {VCCI_2.5_VOLTR} {COM}
set_def {VCCI_3.3_VOLTR} {COM}
set_def {RTG4_MITIGATION_ON} {0}
set_def USE_CONSTRAINTS_FLOW 1
set_def NETLIST_TYPE EDIF
set_name cdh_tsat5_system
set_workdir {C:\Users\Joseph Howarth\Documents\UMSATS\CDH_2019\cdh-tsat5-fork\cdh-tsat5\cdh-tsat5-libero\designer\cdh_tsat5_system}
set_log     {C:\Users\Joseph Howarth\Documents\UMSATS\CDH_2019\cdh-tsat5-fork\cdh-tsat5\cdh-tsat5-libero\designer\cdh_tsat5_system\cdh_tsat5_system_coverage_pr.log}
set_design_state pre_layout
