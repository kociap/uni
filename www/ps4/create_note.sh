curl -s --request POST --header "Content-Type: application/json" --header "authorisation: $1" --data "{ \"title\": \"$2\", \"content\": \"$3\" }" http://localhost:3000/note
echo ""
