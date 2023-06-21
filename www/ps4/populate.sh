./register.sh pkocia pwd > /dev/null
token=$(./login.sh pkocia pwd)
echo $token
./create_note.sh $token "Piotr Kocia's Note" "The note contains a sample text" > /dev/null
./create_note.sh $token "Not Piotr Kocia's Note" "A note that has not been created by Piotr Kocia" > /dev/null
