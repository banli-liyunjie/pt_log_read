version=0.21

if [ ! -d "./out" ]; then
    mkdir "./out"
fi

outfile="pt_log_read"
rmfile="${outfile}*"
if ls ./out/${rmfile} 1> /dev/null 2>&1; then
    rm ./out/${rmfile}
fi

gcc ./main.cpp -o  ./out/${outfile}_${version}.exe -static -O2 -lstdc++
