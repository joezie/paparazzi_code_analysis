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

SrcFileList="Useful_Lists/SrcFileList.txt"
SrcFileStr=""
while IFS= read -r var
do
	SrcFileStr+="$var "
done <"$SrcFileList"

MacroList="Useful_Lists/MacroList.txt"
MacroStr=""
while IFS= read -r var
do
	MacroStr+="-extra-arg=-D$var "
done <"$MacroList"


if [ "$#" -ne 1 ]; then
	echo -e "Usage: bash $0 <dumping option>\n-d: dump ast\n-nd: no dump\n"
	exit 0
fi

if [ "${!#}" == '-d' ]; then
	./BFDictBuilderMain_dump ${SrcFileStr} ${PathStr} ${MacroStr}
elif [ "${!#}" == '-nd' ]; then
	./BFDictBuilderMain_no_dump ${SrcFileStr} ${PathStr} ${MacroStr}
else
	echo "input option -d for dump or -nd for no dump"
fi
