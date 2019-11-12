HeaderFileList="Useful_Lists/HeaderFileList.txt"
HeaderFileStr=""
while IFS= read -r var
do
	HeaderFileStr+="-extra-arg=-I$var "
done <"$HeaderFileList"

PathList="Useful_Lists/DirList.txt"
PathStr=""
while IFS= read -r var
do
	PathStr+="-extra-arg=-I$var "
done <"$PathList"


MacroList="Useful_Lists/MacroList.txt"
MacroStr=""
while IFS= read -r var
do
	MacroStr+="-extra-arg=-D$var "
done <"$MacroList"

if [ "$#" -lt 2 ]; then
	echo -e "Usage: bash BFDictBuilderMain_cmd.sh <src_files> <dumping option>\n-d: dump ast\n-nd: no dump\n"
	exit 0
fi

if [ "${!#}" == '-d' ]; then
	./BFDictBuilderMain_dump $1 ${PathStr} ${MacroStr}

elif [ "${!#}" == '-nd' ]; then
	./BFDictBuilderMain_no_dump $1 ${PathStr} ${MacroStr}

else
	echo "input option -d for dump or -nd for no dump"
fi
