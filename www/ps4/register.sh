curl -s --request POST --header "Content-Type: application/json" --data "{ \"username\": \"$1\", \"password\": \"$2\" }" http://localhost:3000/register
echo ""
