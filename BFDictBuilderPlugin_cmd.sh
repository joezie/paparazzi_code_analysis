HeaderFileList="Useful_Lists/HeaderFileList.txt"
HeaderFileStr=""
while IFS= read -r var
do
	HeaderFileStr+="-Xclang -I -Xclang $var "
done <"$HeaderFileList"

PathList="Useful_Lists/DirList.txt"
PathStr=""
while IFS= read -r var
do
	PathStr+="-Xclang -I -Xclang $var "
done <"$PathList"


MacroList="Useful_Lists/MacroList.txt"
MacroStr=""
while IFS= read -r var
do
	MacroStr+="-Xclang -D -Xclang $var "
done <"$MacroList"

if [ "$#" -lt 2 ]; then
	echo -e "Usage: bash BFDictBuilderPlugin_cmd.sh <src_files> <dumping option>\n-d: dump ast\n-nd: no dump\n"
	exit 0
fi

if [ "${!#}" == '-d' ]; then
	clang -Xclang -load -Xclang ./myplugins/BFDictBuilderPlugin_dump.so -Xclang -add-plugin -Xclang bfdictbuilder $1 ${PathStr} ${MacroStr}

elif [ "${!#}" == '-nd' ]; then
	clang -Xclang -load -Xclang ./myplugins/BFDictBuilderPlugin.so -Xclang -add-plugin -Xclang bfdictbuilder $1 ${PathStr} ${MacroStr}

else
	echo "input option -d for dump or -nd for no dump"
fi
