class CListParser {
  constructor() {
    this.object_list = []
    this.matrix_list = []
  }

  str_to_bool(int) {
    return parseInt(int, 10) ? true : false
  }

  make_object(string) {
    const settings = string.split("-")
    const object = {}
    object["name"] = settings[0];
    object["id"] = settings[1];
    object["collection_id"] = settings[2];
    object["is_collection"] = this.str_to_bool(settings[3]);
    object["is_part_of"] = this.str_to_bool(settings[4]);
    object["is_selected"] = false
    object["is_collapsed"] = false

    object["sub_item"] = []
    return object;
  }

  parse_list(list_as_string) {
    const items = list_as_string.split("|")
    items.pop()

    this.object_list = []

    for (let i = 0; i < items.length; i++) {
      const parent = this.make_object(items[i])

      if (parent.is_collection) {
        for (let j = i + 1; j < items.length; j++) {
          const sub_item = this.make_object(items[j])
          if (sub_item.collection_id == parent.id) {
            parent.sub_item.push(sub_item)
          }
        }
        i += parent.sub_item.length
      }
        
      this.object_list.push(parent)
    }
  }

  open_close(index) {

    const state = this.object_list[index].is_collapsed
    if (state) this.object_list[index].is_collapsed = false
    else this.object_list[index].is_collapsed = true

    console.log(this.object_list)
  }
}