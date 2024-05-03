import flask

import database

app = flask.Flask(__name__)


@app.get("/artworks")
def artworks_list():
    artworks = database.load_artworks()

    return flask.render_template("artworks-list.html", artworks=artworks)


@app.get("/artworks/<int:conf>")
def artwork_display(conf):
    bits = database.num_to_bitarray(conf)
    on_offs = ["on" if bit else "off" for bit in bits]

    return flask.render_template("artwork-display.html", config=on_offs)


@app.delete("/artworks/<int:conf>")
def artwork_delete(conf):
    database.remove_artwork(conf)
    return ""


@app.put("/artworks/<int:conf>/inRandom")
def artwork_random(conf):
    in_random = bool(flask.request.form.get("inRandom"))
    current = database.artwork(conf)
    if not current:
        return 403  # TODO

    current["in_random"] = in_random
    database.add_artwork(conf, current, overwrite=True)

    return ""


@app.route("/<path:filename>")
def serve_static(filename):
    return flask.send_from_directory("static", filename)


@app.route("/")
def index():
    return flask.send_file("static/index.html")


if __name__ == "__main__":
    app.run(debug=True, port=5001)
