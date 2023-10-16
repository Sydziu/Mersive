set -e
./integrationTest > result.txt
diff reference.txt result.txt 
