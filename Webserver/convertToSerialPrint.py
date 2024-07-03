# convert the given html document into a document with the following schema:
# Example:
# <td><a href="/led/an">LED</a></td>
# Turns into:
# client.println("<td><a href=\"/led/an\">LED</a></td>");
# To do this we need to go line by line and add the client.println and also all the " need to be correctly escaped
# Additionally remove any whitespace at the start of every line
 
def convert(document: str):
    split = document.split("\n")
    new_document = ""
    for line in split:
        new_line = line.strip()
        if new_line == "":
            continue
        new_line = new_line.replace("\"", "\\\"")
        new_line = f"client.println(\"{new_line}\");"
        new_document += new_line + "\n"
    return new_document
 
def main():
    with open("Webserver.html", "r") as f:
        document = f.read()
    new_document = convert(document)
    with open("test.cpp", "w") as f:
        f.write(new_document)  
 
if __name__ == "__main__":
    main()