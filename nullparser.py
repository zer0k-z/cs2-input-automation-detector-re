def process_file(filename):
    with open(filename, 'r', encoding='utf-8', errors='ignore') as file:
        lines = [line.strip() for line in file.readlines()]

    valid_lines = [line for line in lines if line in ["null", "over", "under"]]
    # to be used in the spreadsheet
    for line in valid_lines:
        if line == "null":
            print("N")
        if line == "over":
            print("O")
        if line == "under":
            print("U")
            
    # quick (inaccurate) analysis
    null = 0
    overlap = 0
    i = 0
    valid_lines = [line for line in lines if line in ["+left", "-left", "+right", "-right"]]
    while i < len(valid_lines) - 1:
        current = valid_lines[i]
        next_line = valid_lines[i + 1]

        if (current == "-left" and next_line == "+right") or (current == "-right" and next_line == "+left"):
            null += 1
            print(f"Null = {null}, Overlap = {overlap}")

        elif (current == "+right" and next_line == "-left") or (current == "+left" and next_line == "-right"):
            overlap += 1
            print(f"Null = {null}, Overlap = {overlap}")

        i += 1

# copy console output to input.txt
filename = 'input.txt'
process_file(filename)