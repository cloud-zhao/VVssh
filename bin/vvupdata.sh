#!/bin/bash

database='./../data/mydata.db'

function _help(){
	echo "Usage: $0 [hostname|ip|role] [hostname|ip|user|password|role]"
	echo "Ex: $0 hostname=dn1 dn2|12.10.10.12|root|123456|test [update dingle host info]"
	echo "    $0 ip=10.10.10.10 dn2|||mypasswd|role [update dingle ip info]"
	echo "    $0 role=test ||myuser|mypasswd| [cautiously use:update all role info]"
	echo "Don't update option is set to the empty"
}

[ $# != 2 ] && _help

whereid=`echo $1 | awk -F '=' '{print $1}'`
wherevalue=`echo $1 | awk -F '=' '{print $2}'`
host=`echo $2 | awk -F '|' '{print $1}'`
ip=`echo $2 | awk -F '|' '{print $2}'`
user=`echo $2 | awk -F '|' '{print $3}'`
password=`echo $2 | awk -F '|' '{print $4}'`
role=`echo $2 | awk -F '|' '{print $5}'`

sql="updata hostinfo set "

[ ${host:=0} != 0 ] && sql=${sql}"hostname='$host',"
[ ${ip:=0} != 0 ] && sql=${sql}"ip='$ip',"
[ ${user:=0} != 0 ] && sql=${sql}"user='$user',"
[ ${password:=0} != 0 ] && sql=${sql}"password='$password',"
[ ${role:=0} != 0 ] && sql=${sql}"role='$role' "
if [ ${whereid:=0} != 0 ] && [ ${wherevalue:=0} != 0 ]
then
	sql=`echo $sql | sed -e 's/,$/ /'`
	sql=${sql}"where $whereid='$wherevalue';"
	echo $sql
else
	echo "Parameter error"
	exit;
fi

sqlite3 -version >/dev/null 2>&1
if [ $? != 0 ]
then
	echo "Please install sqlite3-clinet"
	yum install -y SQLite3
	[ $? != 0 ] && exit
fi

sqlite3 $database "$sql"
echo "updata sucessful"
