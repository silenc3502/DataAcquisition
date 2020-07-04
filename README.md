# Author
Sanghoon Lee(gcccompil3r@gmail.com)  

# DataAcquisition
C & C++ Based Data Acquisition

# Preparation
sudo apt-get install mysql-server  
sudo apt-get install libmysqlclient-dev  

# How to Test it
sudo mysql -u 'your account' -p 'password'  
mysql> create database cdb;  
mysql> create user 'bitai'@localhost identified by '456123';  
mysql> grant all privileges on \*.\* to 'bitai'@localhost identified by '456123';  
mysql> flush privileges;  
mysql> exit  

# How to compile MySQL Associated Code
gcc mysql_query.c `mysql_config --cflags --libs` -std=c99  
