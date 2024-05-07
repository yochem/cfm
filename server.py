from pathlib import Path

import flask

import database

app = flask.Flask(__name__)


@app.get("/artworks")
def artworks_list() -> str:
    artworks = database.load_artworks()

    return flask.render_template("artworks-list.html", artworks=artworks)


@app.post("/artworks")
def add_artwork() -> flask.Response:
    form = flask.request.form

    name = form.get('display_name')
    if not name:
        return flask.Response(empty_artwork())

    try:
        # get list of 'on'|'off' values and convert to True|False
        config = [form[f'T{num}'] == 'on' for num in range(42)]

        # which can then be converted to an integer
        numeric_config = database.bitarray_to_num(config)

        database.add_artwork(numeric_config, values={
            "display_name": name,
            "in_random": True
        })
    except KeyError:
        return flask.Response(empty_artwork())


    return flask.Response(empty_artwork(), status=201)


@app.get("/artworks/preset/empty")
def empty_artwork() -> str:
    on_offs = ["off"] * 42
    return flask.render_template("led-matrix.html", config=on_offs)


@app.get("/artworks/preset/full")
def full_artwork() -> str:
    on_offs = ["on"] * 42
    return flask.render_template("led-matrix.html", config=on_offs)


@app.get("/artworks/<int:conf>")
def artwork_display(conf: int) -> str:
    bits = database.num_to_bitarray(conf)
    on_offs = ["on" if bit else "off" for bit in bits]


    return flask.render_template(
        "led-matrix.html",
        config=on_offs,
        readonly="readonly" in flask.request.args
    )


@app.delete("/artworks/<int:conf>")
def artwork_delete(conf: int) -> str:
    database.remove_artwork(conf)
    return ""


@app.put("/artworks/<int:conf>/inRandom")
def artwork_random(conf: int):
    in_random = bool(flask.request.form.get("inRandom"))
    current = database.artwork(conf)
    if not current:
        return 403  # TODO

    current["in_random"] = in_random
    database.add_artwork(conf, current, overwrite=True)

    return artwork_display(conf)


@app.route("/<path:filename>")
def serve_static(filename: Path) -> flask.Response:
    return flask.send_from_directory("static", filename)


@app.route("/")
def index() -> flask.Response:
    return flask.send_file("static/select.html")


if __name__ == "__main__":
    app.run(debug=True, port=5001)
