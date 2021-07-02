toupper()
{
    printf "$@\n" | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ
}

name=lib$1
ucname=$(toupper ${name})
file=$2

eval $(awk "/#define ${ucname}_VERSION/ { print \$2 \"=\" \$3 }" "$file")
#eval ${ucname}_VERSION=\$${ucname}_VERSION
eval printf "${name}_VERSION=\$${ucname}_VERSION\\\n"
