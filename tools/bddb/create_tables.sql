# phpMyAdmin MySQL-Dump
# http://phpwizard.net/phpMyAdmin/
#
# Host: localhost Database : hymod_bddb

# (C) Copyright 2001
# Murray Jensen <Murray.Jensen@csiro.au>
# CSIRO Manufacturing and Infrastructure Technology, Preston Lab

# --------------------------------------------------------
#
# Table structure for table 'boards'
#

DROP TABLE IF EXISTS boards;
CREATE TABLE boards (
   serno int(10) unsigned zerofill NOT NULL auto_increment,
   ethaddr char(17),
   date date NOT NULL,
   batch char(32),
   type enum('IO','CLP','DSP','INPUT','ALT-INPUT','DISPLAY') NOT NULL,
   rev tinyint(3) unsigned zerofill NOT NULL,
   location char(64),
   comments text,
   sdram0 enum('32M','64M','128M','256M','512M','1G','2G','4G'),
   sdram1 enum('32M','64M','128M','256M','512M','1G','2G','4G'),
   sdram2 enum('32M','64M','128M','256M','512M','1G','2G','4G'),
   sdram3 enum('32M','64M','128M','256M','512M','1G','2G','4G'),
   flash0 enum('4M','8M','16M','32M','64M','128M','256M','512M','1G'),
   flash1 enum('4M','8M','16M','32M','64M','128M','256M','512M','1G'),
   flash2 enum('4M','8M','16M','32M','64M','128M','256M','512M','1G'),
   flash3 enum('4M','8M','16M','32M','64M','128M','256M','512M','1G'),
   zbt0 enum('512K','1M','2M','4M','8M','16M'),
   zbt1 enum('512K','1M','2M','4M','8M','16M'),
   zbt2 enum('512K','1M','2M','4M','8M','16M'),
   zbt3 enum('512K','1M','2M','4M','8M','16M'),
   zbt4 enum('512K','1M','2M','4M','8M','16M'),
   zbt5 enum('512K','1M','2M','4M','8M','16M'),
   zbt6 enum('512K','1M','2M','4M','8M','16M'),
   zbt7 enum('512K','1M','2M','4M','8M','16M'),
   zbt8 enum('512K','1M','2M','4M','8M','16M'),
   zbt9 enum('512K','1M','2M','4M','8M','16M'),
   zbta enum('512K','1M','2M','4M','8M','16M'),
   zbtb enum('512K','1M','2M','4M','8M','16M'),
   zbtc enum('512K','1M','2M','4M','8M','16M'),
   zbtd enum('512K','1M','2M','4M','8M','16M'),
   zbte enum('512K','1M','2M','4M','8M','16M'),
   zbtf enum('512K','1M','2M','4M','8M','16M'),
   xlxtyp0 enum('XCV300E','XCV400E','XCV600E','XC2V2000','XC2V3000','XC2V4000','XC2V6000','XC2VP2','XC2VP4','XC2VP7','XC2VP20','XC2VP30','XC2VP50','XC4VFX20','XC4VFX40','XC4VFX60','XC4VFX100','XC4VFX140'),
   xlxtyp1 enum('XCV300E','XCV400E','XCV600E','XC2V2000','XC2V3000','XC2V4000','XC2V6000','XC2VP2','XC2VP4','XC2VP7','XC2VP20','XC2VP30','XC2VP50','XC4VFX20','XC4VFX40','XC4VFX60','XC4VFX100','XC4VFX140'),
   xlxtyp2 enum('XCV300E','XCV400E','XCV600E','XC2V2000','XC2V3000','XC2V4000','XC2V6000','XC2VP2','XC2VP4','XC2VP7','XC2VP20','XC2VP30','XC2VP50','XC4VFX20','XC4VFX40','XC4VFX60','XC4VFX100','XC4VFX140'),
   xlxtyp3 enum('XCV300E','XCV400E','XCV600E','XC2V2000','XC2V3000','XC2V4000','XC2V6000','XC2VP2','XC2VP4','XC2VP7','XC2VP20','XC2VP30','XC2VP50','XC4VFX20','XC4VFX40','XC4VFX60','XC4VFX100','XC4VFX140'),
   xlxspd0 enum('6','7','8','4','5','9','10','11','12'),
   xlxspd1 enum('6','7','8','4','5','9','10','11','12'),
   xlxspd2 enum('6','7','8','4','5','9','10','11','12'),
   xlxspd3 enum('6','7','8','4','5','9','10','11','12'),
   xlxtmp0 enum('COM','IND'),
   xlxtmp1 enum('COM','IND'),
   xlxtmp2 enum('COM','IND'),
   xlxtmp3 enum('COM','IND'),
   xlxgrd0 enum('NORMAL','ENGSAMP'),
   xlxgrd1 enum('NORMAL','ENGSAMP'),
   xlxgrd2 enum('NORMAL','ENGSAMP'),
   xlxgrd3 enum('NORMAL','ENGSAMP'),
   cputyp enum('MPC8260(HIP3)','MPC8260A(HIP4)','MPC8280(HIP7)','MPC8560'),
   cpuspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ','300MHZ','333MHZ','366MHZ','400MHZ','433MHZ','466MHZ','500MHZ','533MHZ','566MHZ','600MHZ','633MHZ','666MHZ','700MHZ','733MHZ','766MHZ','800MHZ','833MHZ','866MHZ','900MHZ','933MHZ','966MHZ','1000MHZ','1033MHZ','1066MHZ','1100MHZ','1133MHZ','1166MHZ','1200MHZ','1233MHZ','1266MHZ','1300MHZ','1333MHZ'),
   cpmspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ','300MHZ','333MHZ','366MHZ','400MHZ','433MHZ','466MHZ','500MHZ','533MHZ','566MHZ','600MHZ','633MHZ','666MHZ','700MHZ','733MHZ','766MHZ','800MHZ','833MHZ','866MHZ','900MHZ','933MHZ','966MHZ','1000MHZ','1033MHZ','1066MHZ','1100MHZ','1133MHZ','1166MHZ','1200MHZ','1233MHZ','1266MHZ','1300MHZ','1333MHZ'),
   busspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ','300MHZ','333MHZ','366MHZ','400MHZ','433MHZ','466MHZ','500MHZ','533MHZ','566MHZ','600MHZ','633MHZ','666MHZ','700MHZ','733MHZ','766MHZ','800MHZ','833MHZ','866MHZ','900MHZ','933MHZ','966MHZ','1000MHZ','1033MHZ','1066MHZ','1100MHZ','1133MHZ','1166MHZ','1200MHZ','1233MHZ','1266MHZ','1300MHZ','1333MHZ'),
   hstype enum('AMCC-S2064A','Xilinx-Rockets'),
   hschin enum('0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16'),
   hschout enum('0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16'),
   PRIMARY KEY (serno),
   KEY serno (serno),
   UNIQUE serno_2 (serno)
);

#
# Table structure for table 'log'
#

DROP TABLE IF EXISTS log;
CREATE TABLE log (
   logno int(10) unsigned zerofill NOT NULL auto_increment,
   serno int(10) unsigned zerofill NOT NULL,
   date date NOT NULL,
   details text NOT NULL,
   PRIMARY KEY (logno),
   KEY logno (logno, serno, date),
   UNIQUE logno_2 (logno)
);
