# phpMyAdmin MySQL-Dump
# http://phpwizard.net/phpMyAdmin/
#
# Host: localhost Database : hymod_bddb

# (C) Copyright 2001
# Murray Jensen <Murray.Jensen@cmst.csiro.au>
# CSIRO Manufacturing Science and Technology, Preston Lab

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
   sdram0 enum('32M','64M','128M','256M'),
   sdram1 enum('32M','64M','128M','256M'),
   sdram2 enum('32M','64M','128M','256M'),
   sdram3 enum('32M','64M','128M','256M'),
   flash0 enum('4M','8M','16M','32M','64M'),
   flash1 enum('4M','8M','16M','32M','64M'),
   flash2 enum('4M','8M','16M','32M','64M'),
   flash3 enum('4M','8M','16M','32M','64M'),
   zbt0 enum('512K','1M','2M','4M'),
   zbt1 enum('512K','1M','2M','4M'),
   zbt2 enum('512K','1M','2M','4M'),
   zbt3 enum('512K','1M','2M','4M'),
   zbt4 enum('512K','1M','2M','4M'),
   zbt5 enum('512K','1M','2M','4M'),
   zbt6 enum('512K','1M','2M','4M'),
   zbt7 enum('512K','1M','2M','4M'),
   zbt8 enum('512K','1M','2M','4M'),
   zbt9 enum('512K','1M','2M','4M'),
   zbta enum('512K','1M','2M','4M'),
   zbtb enum('512K','1M','2M','4M'),
   zbtc enum('512K','1M','2M','4M'),
   zbtd enum('512K','1M','2M','4M'),
   zbte enum('512K','1M','2M','4M'),
   zbtf enum('512K','1M','2M','4M'),
   xlxtyp0 enum('XCV300E','XCV400E','XCV600E'),
   xlxtyp1 enum('XCV300E','XCV400E','XCV600E'),
   xlxtyp2 enum('XCV300E','XCV400E','XCV600E'),
   xlxtyp3 enum('XCV300E','XCV400E','XCV600E'),
   xlxspd0 enum('6','7','8'),
   xlxspd1 enum('6','7','8'),
   xlxspd2 enum('6','7','8'),
   xlxspd3 enum('6','7','8'),
   xlxtmp0 enum('COM','IND'),
   xlxtmp1 enum('COM','IND'),
   xlxtmp2 enum('COM','IND'),
   xlxtmp3 enum('COM','IND'),
   xlxgrd0 enum('NORMAL','ENGSAMP'),
   xlxgrd1 enum('NORMAL','ENGSAMP'),
   xlxgrd2 enum('NORMAL','ENGSAMP'),
   xlxgrd3 enum('NORMAL','ENGSAMP'),
   cputyp enum('MPC8260'),
   cpuspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ'),
   cpmspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ'),
   busspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ'),
   hstype enum('AMCC-S2064A'),
   hschin enum('0','1','2','3','4'),
   hschout enum('0','1','2','3','4'),
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
