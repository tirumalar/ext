test="Updating Eyelock.ini... please wait"
echo $test


part1=$(sed -n '/MT9P001.global_gain_val/  p'  /home/root/Eyelock.ini)
part2=$(sed -n '/MT9P001.global_gain_val/  p'  /home/root/EyelockBak.ini)

echo "sed -i -e \"s/$part1/$part2/\" /home/root/Eyelock.ini" > /home/root/replace.cmd

chmod 777 /home/root/replace.cmd
/home/root/replace.cmd