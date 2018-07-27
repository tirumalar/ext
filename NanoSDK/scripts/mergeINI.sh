# /home/root/Eyelock.ini ==> existing ini file, not from new package
# /home/default/Eyelock.ini ==> new default file from tar package
# /home/Eyelock.default ==> existing default file, copied from /home/default/Eyelock.ini before upgrade

# compare default files
diff -wb /home/Eyelock.default /home/default/Eyelock.ini | grep -v '^<\s*$' | grep -v '^>\s*$' > diff_tmp
if [ ! -s diff_tmp ];
    then
	echo "no need to merge parameters in Eyelock.ini"
	rm diff_tmp
	rm /home/Eyelock.default
	exit 0
    fi

# remove lines
grep '^>' diff_tmp | cut -c 2- | while read line; 
    do
        echo "Remove a line : $line"
        sed -i "/${line}/d" /home/root/Eyelock.ini
    done 

# append new lines
grep '^<' diff_tmp | cut -c 2- | tr -d [' '] | while read line; 
    do
        param=$(echo $line | sed 's/=.*//') 
        grep -q "[^;]$param" /home/root/Eyelock.ini
        if [ $? -ne 0 ]
        then
            sed -i -e "$ a ${line}" /home/root/Eyelock.ini
            echo "Append a line : $line"
        fi
    done 
    
# cleanup
rm diff_tmp
rm /home/Eyelock.default
