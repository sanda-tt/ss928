"""MR20 target data model in the radar's native coordinate system."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class RadarTarget:
    """One tracked MR20 target.

    x is the lateral distance/velocity and y is the longitudinal
    distance/velocity, matching the vendor's Windows Python sample.
    """

    target_id: int
    x_distance_m: float
    y_distance_m: float
    x_velocity_mps: float
    y_velocity_mps: float
    status: str

    def to_console(self) -> str:
        return (
            f"Target ID:{self.target_id}\n"
            "Position:\n"
            f" x={self.x_distance_m:.1f}m\n"
            f" y={self.y_distance_m:.1f}m\n\n"
            "Velocity:\n"
            f" vx={self.x_velocity_mps:.1f}m/s\n"
            f" vy={self.y_velocity_mps:.1f}m/s\n\n"
            f"Status:{self.status}"
        )

    def to_csv_fields(self, timestamp: str) -> tuple[str, int, float, float, float, float]:
        """Return exactly the requested log fields: timestamp,id,x,y,vx,vy."""
        return (
            timestamp,
            self.target_id,
            self.x_distance_m,
            self.y_distance_m,
            self.x_velocity_mps,
            self.y_velocity_mps,
        )
