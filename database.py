import datetime
import json
from pathlib import Path
from typing import Any


def load_full() -> Any:
    with open("artworks.json", "r") as f:
        return json.loads(f.read())


def load_artworks() -> Any:
    return load_full()["works"]


def save(data: dict[str, Any], path: str = "artworks.json") -> None:
    save_path = Path(path)
    if save_path.exists():
        # TODO: limit number of backups
        backup_dir = Path("backup")
        backup_dir.mkdir(exist_ok=True)
        timestamp = datetime.datetime.now().isoformat()
        save_path.rename(backup_dir / f"db-{timestamp}.json")

    with open(path, "w") as f:
        f.write(json.dumps(data))


def add_artwork(
    num: int, values: dict[str, Any], overwrite: bool = False, save_to_file: bool = True
) -> None:
    data = load_full()

    if str(num) in data["works"] and not overwrite:
        raise KeyError("configuration already exists")

    data["works"][str(num)] = values

    if save_to_file:
        save(data)


def artwork(num: int) -> Any:
    return load_artworks().get(str(num))


def remove_artwork(num: int, save_to_file: bool = True) -> None:
    data = load_full()
    data["works"].pop(str(num), None)

    if save_to_file:
        save(data)


def num_to_bitarray(num: int) -> list[int]:
    return list(int(bit) for bit in f"{num:042b}")


def bitarray_to_num(bitarray: list[int] | list[bool]) -> int:
    return int("".join(str(int(bit)) for bit in bitarray), base=2)
