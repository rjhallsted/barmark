
items = [...document.querySelectorAll('.uid').entries()].map(x => x[1].innerText);
items = items.map(x => x.split("+")[1]).map(x => parseInt(x, 16));
items = items.sort((a, b) => a - b);

segments = [];
current = {start: items[0], end: items[0]};
for (item in items) {
    if (items[item] == current.end + 1) {
        current.end = items[item];
    } else {
        segments.push(current);
        current = {start: items[item], end: items[item]};
    }
}
segments.push(current);

segments = segments.map(x => {
    start = x.start.toString(16).toUpperCase();
    end = x.end.toString(16).toUpperCase();

    return `{0x${start}, 0x${end}},`;
})

console.log(segments.join("\n"));