if [ "$#" -lt 2 ]; then
	echo -e "Usage: bash run.sh <src_files> <dumping option>\n-d: dump ast\n-nd: no dump\n"
	exit 0
fi

#clang -load ./BFDictBuilderPlugin.so -plugin bfdictbuilder $1

if [ "${!#}" == '-d' ]; then
	clang -Xclang -load -Xclang ./BFDictBuilderPlugin_dump.so -Xclang -add-plugin -Xclang bfdictbuilder $1

elif [ "${!#}" == '-nd' ]; then
	clang -Xclang -load -Xclang ./BFDictBuilderPlugin.so -Xclang -add-plugin -Xclang bfdictbuilder $1

else
	echo "input option -d for dump or -nd for no dump"
fi
