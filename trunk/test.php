#!/usr/bin/php
<?
dl ("./modules/rpm.so");

# rpmqi function
# prototype mixed rpmqi (string package, string queryformat = "")
#
# mapping rpm -qi
#
# if failed rpm query, return RPM_FAILURE code
# if success return follows array
#
# array {
#   NAME
#   VERSION
#   RELEASE
#   EPOCH
#   GROUP
#   SIZE
#   SUMMARY
#   DESCRIPTION
# }
#
# 원하는 값만 리턴을 하고 싶을 경우에는 queryformat 을 이용한다. NAME 과 버전
# 만을 리턴 하고 싶다면 $queryformat = "%{NAME}%{VERSION}" 과 같이 지정을 하면
# 된다. queryformat 이 1개의 값만 리턴할 경우에는 string 으로 반환을 한다.
#
# 실패했을 경우에는 error code 를 리턴하며, rpmlastmsg() 함수로 에러 메세지를
# 확인할 수 있다.
#
#=================================================================================

/*
$package = "aparche";

$qf = "%{NAME}%{VERSION}";

$ret = rpmqi ($pkg, $qf);
#$ret = rpmqi ($pkg);
*/

# rpmsign function
# prototpy rpmsig (string package)
#
# mapping rpm -K command
#
# rpm file 의 sign 가 md5 를 체크한다.
# rpmsig 함수는 name, md5, gpg 의 배열을 리턴 한다.
#
# array {
#    name  => given package name (except path)
#    md5   => md5 check of package. success value 1, failed value 0
#    gpg   => gpg check of package. success value 1, failed value 0
# }
#
# 만약 함수가 실행 실패했을 경우에는 error code 로 반환하며, rpmlastmsg
# 함수로 에러 원인을 확인할 수 있다.
#
#=================================================================================

/*
$pkg = "/usr/src/annyung/RPMS/noarch/oops-test-1.0.0-1.noarch.rpm";
$ret = rpmsig ($pkg);
*/

# rpmdep function
# prototype array rpmdep (string package)
# mapping rpm -qR command
#
# 실패시에 에러코드를 반환하며, rpmlastmsg 함수로 에러 메세지를 구할 수 있음.
#
# 의존성에 대항되는 항목을 array 로 리턴
#=================================================================================

/*
$pkg = "/usr/src/annyung/RPMS/noarch/oops-test-1.0.0-1.noarch.rpm";
#$pkg = "apache";
#$pkg = "x.rpm";

$ret = rpmdep ($pkg);
*/


# rpmfind function
# prototype string rpmfind (string package)
#
# mapping rpm -qf
#
# 실패시에 에러코드를 반환하며, rpmlastmsg 함수로 에러 메세지를 구할 수 있음.
#
#=================================================================================

/*
$_file = "/bin/bash";
#$_file = "/bin/bashs";

$ret = rpmfind ($_file);
*/



# rpmplist function
# prototype array(object) rpmplist (string package, string option)
#
# mapping for rpm -qlv or -qlc or -qld
#
# each array have follows object
#
# ary[]->perm
# ary[]->owner
# ary[]->group
# ary[]->size
# ary[]->date
# ary[]->filename
#
# option is two value that one is "conf" and other is "doc"
# conf returns config file list and doc returns documentation file list
# if option is none, returns all list
#
#=================================================================================

/*
$pkg = "apache";
#$pkg = "y.rpm";

$ret = rpmplist ($pkg, "doc");
*/


# rpmverify function
# prototype array(object) rpmverify (string package)
#
# mapping for rpm -V package
#
# if package is good, return RPM_SUCCESS code
# if package is not installed or failed rpm query, return RPM_FAILURE code
# if package is not good, return array (object) detail code
#
# opject is follows {
#   $ary[]->size     => 설치 후, 파일 사이즈가 변경됨
#   $ary[]->mode     => 설치 후, 퍼미션이 변경됨
#   $ary[]->md5      => 설치 후, md5 checksum 값이 변경됨
#   $ary[]->rdev     => 설치 후, 장치 파일의 속성이 변경됨
#   $ary[]->link     => 설치 후, 심볼릭 링크가 변경 됨
#   $ary[]->user     => 설치 후, owner 가 변경됨
#   $ary[]->group    => 설치 후, group 이 변경됨
#   $ary[]->mtime    => 설치 후, 갱신일이 변경됨
#   $ary[]->missing  => 설치 후, 파일이 삭제 됨
#
#       위의 object 들은 값이 1일 경우 변경이 발생한 경우 임
#
#   $ary[]->type
#
#        conf        => rpm build 시에, config file mark
#        doc         => rpm build 시에, doc file mark
#        ghost       => rpm build 시에, ghost file mark
#        license     => rpm build 시에, license file mark
#        readme      => rpm build 시에, readme file mark
#        general     => 일반 파일
#
#   $ary[]->name     => 위의 속성이 변경된 변경된 파일 이름/경로
# }
# 
#=================================================================================

/*
$pkg = "oops-test";
#$pkg = "apache";
$pkg = "z.rpm";
$ret = rpmverify ($pkg);
*/



# rpminstall function
# prototpye int rpminstall (string package, string mode, string or array option)
#
# mapping rpm -i or -U or -F
#
#  mode : i (install) or U (update) or F (Freshen)
#  option : added rpm option (except -i or -U or -F)
#           for example --force, --nodeps ..
#
#=================================================================================

/*
$pkg = "/usr/src/annyung/RPMS/noarch/oops-test-1.0.0-2.noarch.rpm";
$mode = "F";
#$ret = rpminstall ($pkg);
$ret = rpminstall ($pkg, $mode, $opt);
*/


# rpm erase function
# prototype int rpmerase (string package, string or array option)
#
# mapping rpm -e
#
# option : added erase option ( except -e option )
#          for example, --nodeps
#
#=================================================================================

/*
$pkg = "oops-test";
$ret = rpmerase ($pkg, $opt);
*/

# rpm list function
# prototype array rpmlist (string package)
#
# mapping rpm -q package or rpm -qa
#
# if not package, option is -qa
#
# if package is exists, retun value is bool ( 0 true, 1 fail )
# if not package, retun value array of all isntalled package list
#
#=================================================================================

#$pkg = "apache";
$ret = rpmlist ($pkg);


print_r ($ret);

echo "\n\n\n";
printf ("## Return Code : %s\n", $ret);
printf ("## lasterr =>%s\n", rpmlastmsg());
printf ("## RPM_SUCCESS : %d\n", RPM_SUCCESS);
printf ("## RPM_FAILURE : %d\n", RPM_FAILURE);
?>
