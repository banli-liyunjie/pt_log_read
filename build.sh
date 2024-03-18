version=0.40

if [ ! -d "./out" ]; then
    mkdir "./out"
fi

outfile="pt_log_read"
rmfile="${outfile}*"
if ls ./out/${rmfile} 1> /dev/null 2>&1; then
    rm ./out/${rmfile}
fi
g++ -c ./result_print/print.cpp -o ./out/print.o -O2
g++ -c ./main.cpp -o ./out/main.o -O2
g++ -static ./out/print.o ./out/main.o -o ./out/${outfile}_${version}.exe

rm ./out/print.o
rm ./out/main.o
