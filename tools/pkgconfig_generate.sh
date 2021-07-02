#!/bin/sh

. ./config.sh

shortname=$1
name=lib${shortname}
fullname=${name}${build_suffix}
comment=$2
libs=$(eval printf "\$extralibs\\\n")
deps=$(eval printf "\$${shortname}_deps\\\n")

for dep in $deps
do
    depname=lib${dep}
    fulldepname=${depname}${build_suffix}
    . ${depname}/${depname}.version
    depversion=$(eval printf "\$${depname}_VERSION\\\n")
    requires="$requries ${fulldepname} >= ${depversion}, "
done
requires=${requires%, }

version=$(grep ${name}_VERSION= lib/${name}.version | cut -d= -f2)

cat <<EOF >lib/$fullname.pc
prefix=$prefix
exec_prefix=\${prefix}
libdir=$libdir
includedir=$incdir

Name: $fullname
Description: $comment
Version: $version
Requires: $(printf "$requires")
Requires.private: $(printf "$requires")
Conflicts:
Libs: -L\${libdir} -l${fullname#lib} $(printf "$libs")
Libs.private: $(printf "$libs")
Cflags: -I\${includedir}
EOF

mkdir -p doc/examples/pc-uninstalled
includedir=${source_path}
[ "$includedir" = . ] && includedir="\${pcfiledir}/../../.."
cat <<EOF >doc/examples/pc-uninstalled/${name}-uninstalled.pc
prefix=
exec_prefix=
libdir=\${pcfiledir}/../../../$name
includedir=${source_path}

Name: $fullname
Description: $comment
Version: $version
Requires: $requires
Conflicts:
Libs: -L\${libdir} -l${fullname#lib} $(printf "$libs")
Cflags: -I\${includedir}
EOF
