const object_list = new CListParser();

const testing = (value) => {
  console.log("FromFile: " + value)
}

const list_form_c = "fantasy-0-0-1-0|pointer_001-1-0-0-1|cart_001-2-0-0-1|log_004-3-0-0-1|log_003-4-0-0-1|log_001-5-0-0-1|jug_002-6-0-0-1|jug_001-7-0-0-1|holder_001-8-0-0-1|fabulous_tree_001-9-0-0-1|fabulous_mushroom_004-10-0-0-1|fabulous_mushroom_003-11-0-0-1|fabulous_mushroom_002-12-0-0-1|fabulous_mushroom_001-13-0-0-1|big_fabulous_tree_001-14-0-0-1|tree_001-15-0-0-1|fir_001-16-0-0-1|cactus_001-17-0-0-1|stall_001-18-0-0-1|barrel_001-19-0-0-1|table_001-20-0-0-1|log_002-21-0-0-1|house_003-22-0-0-1|house_002-23-0-0-1|house_001-24-0-0-1|box_001.001-25-0-0-1|stall_table_001-26-0-0-1|plate_001-27-0-0-1|jug_003-28-0-0-1|jug_005-29-0-0-1|jug_004-30-0-0-1|plate_003-31-0-0-1|plate_002-32-0-0-1|bag_001-33-0-0-1|bucket_001-34-0-0-1|box_003-35-0-0-1|box_002-36-0-0-1|box_001-37-0-0-1|bag_002-38-0-0-1|bag_003-39-0-0-1|bag_004-40-0-0-1|crane_rope_001-41-0-0-1|crane_wheel_002-42-0-0-1|crane_wheel_001-43-0-0-1|crane_hook_001-44-0-0-1|crane_001-45-0-0-1|cars-46-46-1-0|Police_Plane.008-47-46-0-1|Taxi_Plane.026-48-46-0-1|SUV_Plane.023-49-46-0-1|Pickup_Plane.009-50-46-0-1|Car_Plane.032-51-46-0-1|Van_Plane.020-52-46-0-1|Limousine_Plane.018-53-46-0-1|church-54-54-0-0|";
object_list.parse_list(list_form_c);

const open_close_col = (index) => {
  object_list.open_close(index)
  updade_item_hierarchy()
}

const make_item = (settings, index) => {
  const item_div = document.createElement("div")
  item_div.classList.add("item")

  if (settings.is_collection) {
    const col_span = document.createElement("span")
    col_span.classList.add("collection_control")
    if (settings.is_collapsed) col_span.classList.add("open_collection")
    else col_span.classList.add("hide_collection")
    col_span.setAttribute("onclick", "open_close_col(" + index + ")")
    col_span.innerText = ">"
    item_div.appendChild(col_span)
  }

  if (settings.is_part_of) {
    item_div.classList.add("sub_item")
    item_div.innerHTML += "<div class='sub_item_line'></div>"
  }

  item_div.innerHTML += "<span>" + settings.name + "</span>"
  return item_div
}

const updade_item_hierarchy = () => {
  const list = object_list.object_list;
  const item_hierarchy = document.getElementById("item_box")
  item_hierarchy.innerHTML = ""

  for (let i = 0; i < list.length; i++) {
    const element = list[i];
    console.log(element)

    const item = make_item(element, i)
    item_hierarchy.appendChild(item)

    if (element.sub_item.length > 0 && !element.is_collapsed) {
      for (let j = 0; j < element.sub_item.length; j++) {
        const sub_item = element.sub_item[j];

        const item = make_item(sub_item, j)
        item_hierarchy.appendChild(item)
      }
    }
  }
}
updade_item_hierarchy()

const upload_model = () => {
  const reader = new FileReader()


  const files = document.getElementById('myfile').files
  const file = files[0]

  reader.onload = () => {
    const uint8Array = new Uint8Array(reader.result)
    FS.writeFile("resources/models/" + file.name, uint8Array)

    c_upload_model("resources/models/" + file.name, "resources/models/cars.png", "woowing")
    const list_form_c = c_get_model_list();
    object_list.parse_list(list_form_c);
    updade_item_hierarchy()
  };

  reader.readAsArrayBuffer(file);
}

const set_edit_tool = (index) => {
  const tools = document.getElementsByClassName("edit_pos");
  for (let i = 0; i < tools.length; i++) {
    const element = tools[i];
    element.classList.remove("active")
  }

  tools[index].classList.add("active")
}

