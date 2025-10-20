# engine_connect5.py
import engine_connect5_core

def get_best_move(board_str: str, time_ms: int = 1000):
    """
    board_str: 225-char string, row-major: '.' empty, 'B' black, 'W' white
    time_ms: milliseconds to search
    Returns: dict { "row": int, "col": int, "eval": int , "depth": int}
    """
    try:
        res = engine_connect5_core.get_best_move_cpp(board_str, time_ms)
        return {
            "row": int(res["row"]),
            "col": int(res["col"]),
            "eval": int(res.get("eval", 0)),
            "depth": int(res.get("depth", 0))
        }
    except Exception as e:
        print("Engine error:", e)
        return {"row": -1, "col": -1, "eval": 0, "depth": 0}
