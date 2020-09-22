select t.hometown, avg(level)
from Trainer t, CatchedPokemon c
where t.id = c.owner_id
group by t.hometown
order by avg(level);