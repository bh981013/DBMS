select avg(level)
from CatchedPokemon
where owner_id in (select leader_id from Gym);