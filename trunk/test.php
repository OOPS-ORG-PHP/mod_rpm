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
# ���ϴ� ���� ������ �ϰ� ���� ��쿡�� queryformat �� �̿��Ѵ�. NAME �� ����
# ���� ���� �ϰ� �ʹٸ� $queryformat = "%{NAME}%{VERSION}" �� ���� ������ �ϸ�
# �ȴ�. queryformat �� 1���� ���� ������ ��쿡�� string ���� ��ȯ�� �Ѵ�.
#
# �������� ��쿡�� error code �� �����ϸ�, rpmlastmsg() �Լ��� ���� �޼�����
# Ȯ���� �� �ִ�.
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
# rpm file �� sign �� md5 �� üũ�Ѵ�.
# rpmsig �Լ��� name, md5, gpg �� �迭�� ���� �Ѵ�.
#
# array {
#    name  => given package name (except path)
#    md5   => md5 check of package. success value 1, failed value 0
#    gpg   => gpg check of package. success value 1, failed value 0
# }
#
# ���� �Լ��� ���� �������� ��쿡�� error code �� ��ȯ�ϸ�, rpmlastmsg
# �Լ��� ���� ������ Ȯ���� �� �ִ�.
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
# ���нÿ� �����ڵ带 ��ȯ�ϸ�, rpmlastmsg �Լ��� ���� �޼����� ���� �� ����.
#
# �������� ���׵Ǵ� �׸��� array �� ����
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
# ���нÿ� �����ڵ带 ��ȯ�ϸ�, rpmlastmsg �Լ��� ���� �޼����� ���� �� ����.
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
#   $ary[]->size     => ��ġ ��, ���� ����� �����
#   $ary[]->mode     => ��ġ ��, �۹̼��� �����
#   $ary[]->md5      => ��ġ ��, md5 checksum ���� �����
#   $ary[]->rdev     => ��ġ ��, ��ġ ������ �Ӽ��� �����
#   $ary[]->link     => ��ġ ��, �ɺ��� ��ũ�� ���� ��
#   $ary[]->user     => ��ġ ��, owner �� �����
#   $ary[]->group    => ��ġ ��, group �� �����
#   $ary[]->mtime    => ��ġ ��, �������� �����
#   $ary[]->missing  => ��ġ ��, ������ ���� ��
#
#       ���� object ���� ���� 1�� ��� ������ �߻��� ��� ��
#
#   $ary[]->type
#
#        conf        => rpm build �ÿ�, config file mark
#        doc         => rpm build �ÿ�, doc file mark
#        ghost       => rpm build �ÿ�, ghost file mark
#        license     => rpm build �ÿ�, license file mark
#        readme      => rpm build �ÿ�, readme file mark
#        general     => �Ϲ� ����
#
#   $ary[]->name     => ���� �Ӽ��� ����� ����� ���� �̸�/���
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
