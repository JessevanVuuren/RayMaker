let pos_matrix_display = JSON.parse(localStorage.getItem("pos_matrix_display"))
if (pos_matrix_display == null) pos_matrix_display = { "y_delta": 0, "y_last_pos": 0, "x_delta": 0, "x_last_pos": 0, "selected": false }

let pos_item_hierarchy = JSON.parse(localStorage.getItem("pos_item_hierarchy"))
if (pos_item_hierarchy == null) pos_item_hierarchy = { "y_delta": 0, "y_last_pos": 0, "x_delta": 0, "x_last_pos": 0, "selected": false }

const matrix_display = document.getElementById("matrix_display")
const item_hierarchy = document.getElementById("item_hierarchy")

matrix_display.style.bottom = pos_matrix_display.y_last_pos + "px"
matrix_display.style.right = pos_matrix_display.x_last_pos + "px"

item_hierarchy.style.top = -pos_item_hierarchy.y_last_pos + "px"
item_hierarchy.style.right = pos_item_hierarchy.x_last_pos + "px"



let mouse_down = false
document.addEventListener("mousedown", (e) => {
  if (e.toElement.classList.contains("window_title")) {
    pos_matrix_display.selected = e.toElement.dataset.parent == "matrix_display"
    pos_item_hierarchy.selected = e.toElement.dataset.parent == "item_hierarchy"

    mouse_down = true
    pos_matrix_display.y_delta = e.clientY
    pos_matrix_display.x_delta = e.clientX

    pos_item_hierarchy.y_delta = e.clientY
    pos_item_hierarchy.x_delta = e.clientX
  }
})
document.addEventListener("mouseup", (e) => {
  mouse_down = false
  const matrix_bottom = getComputedStyle(matrix_display).bottom;
  const matrix_right = getComputedStyle(matrix_display).right;
  pos_matrix_display.y_last_pos = parseInt(matrix_bottom.slice(0, -2), 10)
  pos_matrix_display.x_last_pos = parseInt(matrix_right.slice(0, -2), 10)

  const item_top = getComputedStyle(item_hierarchy).top;
  const item_right = getComputedStyle(item_hierarchy).right;
  pos_item_hierarchy.y_last_pos = -parseInt(item_top.slice(0, -2), 10)
  pos_item_hierarchy.x_last_pos = parseInt(item_right.slice(0, -2), 10)

  localStorage.setItem("pos_matrix_display", JSON.stringify(pos_matrix_display))
  localStorage.setItem("pos_item_hierarchy", JSON.stringify(pos_item_hierarchy))

})


document.addEventListener("mousemove", (e) => {
  if (!mouse_down) return

  if (pos_matrix_display.selected) {
    const pos_matrix_y = pos_matrix_display.y_delta - e.clientY
    const pos_matrix_x = pos_matrix_display.x_delta - e.clientX
    matrix_display.style.bottom = (pos_matrix_display.y_last_pos + pos_matrix_y) + "px";
    matrix_display.style.right = (pos_matrix_display.x_last_pos + pos_matrix_x) + "px";
  }

  if (pos_item_hierarchy.selected) {
    const pos_items_y = pos_matrix_display.y_delta - e.clientY
    const pos_items_x = pos_matrix_display.x_delta - e.clientX
    item_hierarchy.style.top = (-(pos_item_hierarchy.y_last_pos + pos_items_y)) + "px";
    item_hierarchy.style.right = (pos_item_hierarchy.x_last_pos + pos_items_x) + "px";
  }
})

const open_hide = (item, button) => {
  const element = document.getElementById(item)
  const svg = document.getElementById(button)
  const state = element.classList.contains("window_hide")
  if (state) {
    element.classList.remove("window_hide")
    svg.src = "./svg/collapse_open.svg"
  }
  else {
    element.classList.add("window_hide")
    svg.src = "./svg/collapse_close.svg"
  }
}